#include <Melopero_AMG8833.h>
#include "VirtualPanel.h"

Melopero_AMG8833 sensor;

bool  PanelInit = false; 
bool  Power = false; 
bool  InfoPanel = false;
int   statusCode = 0; 
float InterpolatePixel[10][10]; 
float hightemp = 0.0; 
float lowtemp  = 0.0; 
float centertemp = 0.0; 
float ScaleTop = 25.0; 
float ScaleBottom = 15.0; 
float ScaleStep = 0; 
byte   ScaleOffset = 30;
byte   hoffset = ((255 - (16 * 8)) / 2) + 8 + 35; 
byte   voffset = ((220 - (16 * 8)) / 2) + 8; 
enum  ImageMode { RAW, IP29, IP64 };
ImageMode ImageModeSelect = RAW;
enum  StillMode { OFF, GET, WRITE, SHOW};
StillMode Still = OFF;
byte  ip = 1;
byte  px = 16;
byte  clickx = 0;
byte  clicky = 0;
bool  mirror = false;

void setup() {
  Panel.begin(); 
  while(!PanelInit) Panel.receive(); 
  Wire.begin(); 
  InitAMG8833();
}

void loop() {
  Panel.receive(); 

  if(Power) 
    SensorData();  
}
