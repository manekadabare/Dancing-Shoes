#include "SPI.h"
#include "Adafruit_NeoPixel.h"
#include <Wire.h> 
#define Addr 0x53

const int groundPin = 19;
const int powerPin = 18;

// Number of RGB LEDs in 2 strands
int nLEDs = 18;
int nLEDs2 = 11;

int dataPin  = 2;
int dataPin2  = 3;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(nLEDs, dataPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(nLEDs2, dataPin2, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show();
  
  strip2.begin();
  strip2.show();
  pinMode(groundPin,OUTPUT);
  pinMode(powerPin, OUTPUT);
  digitalWrite(groundPin,LOW);
  digitalWrite(powerPin,HIGH);
  Wire.begin();  
  Serial.begin(9600);    
  Wire.beginTransmission(Addr);  
  Wire.write(0x2C);  
  Wire.write(0x0A);  
  Wire.endTransmission();      
  Wire.beginTransmission(Addr);   
  Wire.write(0x2D);   
  Wire.write(0x08);    
  Wire.endTransmission();    
  Wire.beginTransmission(Addr);  
  Wire.write(0x31);  
  Wire.write(0x08);   
  Wire.endTransmission();  
}

void loop() {
  int forceReading1=analogRead(A0);
  int forceReading2=analogRead(A1);
  
  //iterate throught the LEDs(only half way because the shoe is a mirror image
  for (int i=0; i < strip.numPixels()/2; i++) {
      //set the current LED and its mirrored pair to a combination of the front and back force sensor readings, proportional to their distance from each sensor
      strip.setPixelColor(i, getColorFromForce((forceReading1*i+forceReading2*((strip.numPixels()/2)-i-1))/10));
      strip.setPixelColor(strip.numPixels()-i-1, getColorFromForce((forceReading1*i+forceReading2*((strip.numPixels()/2)-i-1))/10));
    }
    //send the new colors to the strip
  strip.show();

  unsigned int data[6];  
  for(int i = 0; i < 6; i++){        
    Wire.beginTransmission(Addr);       
    Wire.write((50 + i));       
    Wire.endTransmission();            
    Wire.requestFrom(Addr, 1);            
    if(Wire.available() == 1){      
      data[i] = Wire.read();    
    }  
  }  
    
// Convert the data to 10-bits  
  int xAccl = (((data[1] & 0x03) * 256) + data[0]);  
  if(xAccl > 511){    
    xAccl -= 1024;  
  }  
  int yAccl = (((data[3] & 0x03) * 256) + data[2]);  
  if(yAccl > 511){    
    yAccl -= 1024;  
  }  
  int zAccl = (((data[5] & 0x03) * 256) + data[4]);  
  if(zAccl > 511){    
    zAccl -= 1024;  
  }    

  if (xAccl > yAccl && xAccl > zAccl){
    colorWipe(strip2.Color(255,0,0),50);
  }

  if (yAccl > xAccl && yAccl> zAccl){
    colorWipe(strip2.Color(0,255,0),50);
  }

  if (zAccl > xAccl && zAccl > yAccl){
    colorWipe(strip2.Color(0,0,255),50);
  }
}

void colorWipe(uint32_t c, uint8_t wait){
  for(uint16_t i = 0; i < strip.numPixels(); i++){
    strip2.setPixelColor(i,c);
    strip2.show(); 
  }
}

//helper function that shifts and scales the force reading to a range of 0-384(for the color wheel function)
uint32_t getColorFromForce(int force){
  //the shift and scaling factors were found experimentally
  int color=(force-384)/1.3;
  if (color>384){
    color=384;
  }
  else if (color<0){
    color=0;
  }
  //return the color that matches the force
  return Wheel(color);
}

//Input a value 0 to 384 to get a color value.
//The colours are a transition b - g -r - back to b
uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //blue down
      g = WheelPos % 128;      // Green up
      b = 0;                  //red off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //red up
      r = 0;                  //blue off
      break; 
    case 2:
      r = 127 - WheelPos % 128;  //red down 
      b = WheelPos % 128;      //blue up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}
