void InitAMG8833() {
  sensor.initI2C();
  statusCode = sensor.resetFlagsAndSettings();
  if(statusCode != 0) 
    Panel.sendf(MonitorLog, F("reset status %s"), sensor.getErrorDescription(statusCode));
  statusCode = sensor.setFPSMode(FPS_MODE::FPS_10);
  if(statusCode != 0)
    Panel.sendf(MonitorLog, F("frame speed status %s"), sensor.getErrorDescription(statusCode));
  statusCode = sensor.setMode(DEVICE_MODE::SLEEP);
  if(statusCode != 0)
    Panel.sendf(MonitorLog, F("sleep mode status %s"), sensor.getErrorDescription(statusCode));
}

void SensorData() {
  if(Still == OFF || Still == GET) {
    statusCode = sensor.updateThermistorTemperature();
    if(statusCode != 0) 
      Panel.sendf(MonitorLog, F("thermistor update status %s"), sensor.getErrorDescription(statusCode));

    statusCode = sensor.updatePixelMatrix();
    if(statusCode != 0) 
      Panel.sendf(MonitorLog, F("matrix update status %s"), sensor.getErrorDescription(statusCode));
    
    if(Still == GET) 
      Still = WRITE;
  }
  
  if(Still == OFF || Still == WRITE) {
    if(Still == WRITE) Panel.send(Led_7, F("$ORANGE"));
    hightemp = -500.0;
    lowtemp =   500.0;
    
    if(ImageModeSelect == RAW) 
      OutputRawGrid();
    if(ImageModeSelect == IP29) 
      InterpolateGrid();
    if(ImageModeSelect == IP64) 
      InterpolateGrid();
        
    Panel.sendf(Display_1, F("thermistor ref. %s°C"), _FString(sensor.thermistorTemperature,4,2));
    Panel.send(GraphText, _Point(ScaleOffset,210));
    Panel.sendf(GraphText, F("▲%s°C"), _FString(hightemp,4,1));
    Panel.send(GraphText, _Point(ScaleOffset,23));
    Panel.sendf(GraphText, F("▽%s°C"), _FString(lowtemp,4,1));
    Panel.send(GraphText, _Point(ScaleOffset,40));
    Panel.send(GraphText,  F("$YELLOW"));  
    Panel.sendf(GraphText, F("◯%s°C"), _FString(centertemp,4,1));
    Panel.send(GraphText,  F("$WHITE"));
    if(Still == WRITE) {
      Still = SHOW;
      Panel.send(Led_7, F("$GREEN")); 
    }
  }
  
  if(Still == SHOW) {}
}


void OutputRawGrid() { 
  byte dispe = byte (7 * 16); 

  Panel.send(GraphDrawPixel, F("$16PX")); 
  SetColor(0.0, true); 
  
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      SetColor(sensor.pixelMatrix[x][y], false); 
      if(sensor.pixelMatrix[x][y] > hightemp) 
        hightemp = sensor.pixelMatrix[x][y]; 
      if(sensor.pixelMatrix[x][y] < lowtemp) 
        lowtemp = sensor.pixelMatrix[x][y];
      if(x==4 && y==3)
        centertemp = sensor.pixelMatrix[x][y]; 

      byte dispx = byte(y * 16);
      byte dispy = byte(x * 16);
      if(mirror) 
        dispx = dispe - dispx; 
      Panel.send(GraphDrawPixel, _Point(byte(hoffset + dispx), byte(voffset + dispy))); 
      if(x==4 && y==3) {
        clickx = 154;
        clicky = 118;
        if(mirror) 
          clickx += 16;
        Panel.send(GraphDrawCircle, _Circle(clickx, clicky, 5));
      }
    }
  }
}


void InterpolateGrid() {
  byte ccx =   byte ((((7 * ip * px) + 1) / 2) + hoffset); 
  byte ccy =   byte ((((7 * ip * px) + 1) / 2) + voffset);
  byte dispe = byte (7 * ip * px); 

  clickx = ccx; 
  clicky = ccy;

  if(px==3)
    Panel.send(GraphDrawPixel, F("$3PX")); 
  if(px==4)
    Panel.send(GraphDrawPixel, F("$4PX")); 
  
  SetColor(0.0, true); 
 
  for(int y = 0; y < 7; y++)
    for(int x = 0; x < 7; x++) {
      
      InterpolatePixel [0] [0] = sensor.pixelMatrix[x][y];
      InterpolatePixel[ip] [0] = sensor.pixelMatrix[x+1][y];
      InterpolatePixel [0][ip] = sensor.pixelMatrix[x][y+1];
      InterpolatePixel[ip][ip] = sensor.pixelMatrix[x+1][y+1];
      
      for(int ix = 1; ix < ip; ix++) { 
        InterpolatePixel[ix][0] = ((InterpolatePixel[0][0] * (ip - ix)) + (InterpolatePixel[ip][0] * ix)) / ip;
        InterpolatePixel[ix][ip] = ((InterpolatePixel[0][ip] * (ip - ix)) + (InterpolatePixel[ip][ip] * ix)) / ip;
      }
      for(int ix = 0; ix <= ip; ix++) {
        for(int iy = 1; iy < ip; iy++) {
          InterpolatePixel[ix][iy] = ((InterpolatePixel[ix][0] * (ip - iy)) + (InterpolatePixel[ix][ip] * iy)) / ip;
        }
      
      int xip, yip;
      if(x==6) 
        xip = ip+1; 
      else 
        xip = ip; 
      if(y==6) 
        yip = ip+1; 
      else yip = ip;
      
      for (int ix = 0; ix < xip; ix++) { 
        for (int iy = 0; iy < yip; iy++) {
          SetColor(InterpolatePixel[ix][iy], false); 
          byte dispx = byte (y * ip * px) + (iy * px);
          byte dispy = byte (x * ip * px) + (ix * px);
          if(mirror) 
            dispx = dispe - dispx;
          
          Panel.send(GraphDrawPixel, _Point(byte(hoffset + dispx), byte(voffset + dispy)));
          if(x==3 && y==3 && ix==ip/2 && iy==ip/2) 
            centertemp = InterpolatePixel[ix][iy];
          if(x==3 && y==3) 
            Panel.send(GraphDrawCircle, _Circle(ccx, ccy, 5)); 
        }
      }
      if(sensor.pixelMatrix[x][y] > hightemp) 
        hightemp = sensor.pixelMatrix[x][y]; 
      if(sensor.pixelMatrix[x][y] < lowtemp) 
        lowtemp = sensor.pixelMatrix[x][y]; 
    }
    if(Panel.delay(1, false)) 
      return;
    }
}

void SetColor(float pixel, bool reset) {
  const byte tmin = 50;
  static byte PrevColor = 255;
  byte CurrColor = 0;

  if(reset) { 
     PrevColor = 255;
     return;
  }

  if( pixel + tmin < ScaleBottom + tmin) 
    CurrColor = 0;
  else if ( pixel + tmin > ScaleTop + tmin) 
    CurrColor = 7;
  else 
    CurrColor = (((pixel + tmin) - (ScaleBottom + tmin)) / ScaleStep) + 1;

  if(PrevColor != CurrColor) {
    PrevColor = CurrColor;
    if (CurrColor==0) 
      Panel.send(GraphDrawPixel, F("$DBLUE")); 
    else if(CurrColor==1) 
      Panel.send(GraphDrawPixel, F("$DPURPLE"));
    else if(CurrColor==2) 
      Panel.send(GraphDrawPixel, F("$BLUE"));
    else if(CurrColor==3) 
      Panel.send(GraphDrawPixel, F("$LBLUE"));
    else if(CurrColor==4) 
      Panel.send(GraphDrawPixel, F("$GREEN"));
    else if(CurrColor==5) 
      Panel.send(GraphDrawPixel, F("$YELLOW"));
    else if(CurrColor==6) 
      Panel.send(GraphDrawPixel, F("$ORANGE"));
    else if(CurrColor==7) 
      Panel.send(GraphDrawPixel, F("$RED"));
  }
}

