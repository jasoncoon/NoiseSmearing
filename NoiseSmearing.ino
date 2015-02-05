#include<SmartMatrix.h>
#include<FastLED.h>

#define kMatrixWidth  32
#define kMatrixHeight 32

byte CentreX =  (kMatrixWidth / 2) - 1;
byte CentreY = (kMatrixHeight / 2) - 1;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[kMatrixWidth * kMatrixHeight];
CRGB leds2[kMatrixWidth * kMatrixHeight];

// The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS 1

uint32_t x[NUM_LAYERS];
uint32_t y[NUM_LAYERS];
uint32_t z[NUM_LAYERS];
uint32_t scale_x[NUM_LAYERS];
uint32_t scale_y[NUM_LAYERS];

uint8_t noise[NUM_LAYERS][kMatrixWidth][kMatrixHeight];

uint8_t noisesmoothing;

void setup() {

  Serial.begin(115200);

  LEDS.addLeds<SMART_MATRIX>(leds,NUM_LEDS);

  FastLED.setDither(0);

  pSmartMatrix->setColorCorrection(cc48);

  BasicVariablesSetup();
}

void loop() {
  
  //MultipleStream();
  //MultipleStream2();
  //MultipleStream3();
  //MultipleStream4();
  //MultipleStream5();
  MultipleStream8();
  FastLED.show();
  
}


void BasicVariablesSetup() {

  noisesmoothing = 200;
  for(int i = 0; i < NUM_LAYERS; i++) {
    x[i] = random16();
    y[i] = random16();
    z[i] = random16();
    scale_x[i] = 6000;
    scale_y[i] = 6000;
  }
}

uint16_t XY( uint8_t x, uint8_t y) {
  uint16_t i;
  i = (y * kMatrixWidth) + x;
  return i;
}

void DimAll(byte value)  
{
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}


void FillNoise(byte layer) {

  for(uint8_t i = 0; i < kMatrixWidth; i++) {

    uint32_t ioffset = scale_x[layer] * (i-CentreX);

    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint32_t joffset = scale_y[layer] * (j-CentreY);

      byte data = inoise16(x[layer] + ioffset, y[layer] + joffset, z[layer]) >> 8;

      uint8_t olddata = noise[layer][i][j];
      uint8_t newdata = scale8( olddata, noisesmoothing ) + scale8( data, 256 - noisesmoothing );
      data = newdata;


      noise[layer][i][j] = data;
    }
  }
}

void MoveX(byte delta) {

  //CLS2();

  for(int y = 0; y < kMatrixHeight; y++) {
  
    for(int x = 0; x < kMatrixWidth-delta; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta,y)];
    }
    for(int x = kMatrixWidth-delta; x < kMatrixWidth; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta-kMatrixWidth,y)];
    } 
  }
  

  //CLS();

  // write back to leds
  for(uint8_t y = 0; y < kMatrixHeight; y++) {
    for(uint8_t x = 0; x < kMatrixWidth; x++) {
      leds[XY(x,y)] = leds2[XY(x,y)];
    }
  }
}


void MoveY(byte delta) {

  //CLS2();

  for(int x = 0; x < kMatrixWidth; x++) {
  
    for(int y = 0; y < kMatrixHeight-delta; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta)];
    }
    for(int y = kMatrixHeight-delta; y < kMatrixHeight; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta-kMatrixHeight)];
    } 
  }
  

  //CLS();

  // write back to leds
  for(uint8_t y = 0; y < kMatrixHeight; y++) {
    for(uint8_t x = 0; x < kMatrixWidth; x++) {
      leds[XY(x,y)] = leds2[XY(x,y)];
    }
  }
}

void MoveFractionalNoiseX() {
  
  // move delta pixelwise
  for(int y = 0; y < kMatrixHeight; y++) {
    
  uint16_t amount = noise[0][0][y]*16;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);

    for(int x = 0; x < kMatrixWidth-delta; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta,y)];
    }
    for(int x = kMatrixWidth-delta; x < kMatrixWidth; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta-kMatrixWidth,y)];
    } 
  }
  //CopyBack();
  
  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for(uint8_t y = 0; y < kMatrixHeight; y++) {
    
  uint16_t amount = noise[0][0][y]*16;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);
    
    for(uint8_t x = 1; x < kMatrixWidth; x++) {
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x-1, y)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;
      

      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    } 
    
    PixelA = leds2[XY(0, y)];
    PixelB = leds2[XY(kMatrixWidth-1, y)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
      /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
      */
      

    leds[XY(0, y)] = PixelA + PixelB;
    
  }

}


void MoveFractionalNoiseY() {
  
  // move delta pixelwise
  for(int x = 0; x < kMatrixWidth; x++) {
    
  uint16_t amount = noise[0][x][0]*16;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);

    for(int y = 0; y < kMatrixWidth-delta; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta)];
    }
    for(int y = kMatrixWidth-delta; y < kMatrixWidth; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta-kMatrixWidth)];
    } 
  }
  //CopyBack();
  
  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for(uint8_t x = 0; x < kMatrixHeight; x++) {
    
  uint16_t amount = noise[0][x][0]*16;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);
    
    for(uint8_t y = 1; y < kMatrixWidth; y++) {
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x, y-1)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;
      

      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    } 
    
    PixelA = leds2[XY(x, 0)];
    PixelB = leds2[XY(x, kMatrixWidth-1)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
      /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
      */
      

    leds[XY(x, 0)] = PixelA + PixelB;
    
  }

}


void MoveFractionalNoiseX2() {
  
  
  
  // move delta pixelwise
  for(int y = 0; y < kMatrixHeight; y++) {
    
  uint16_t amount = noise[0][0][y]*4;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);

    for(int x = 0; x < kMatrixWidth-delta; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta,y)];
    }
    for(int x = kMatrixWidth-delta; x < kMatrixWidth; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta-kMatrixWidth,y)];
    } 
  }
  //CopyBack();
  
  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for(uint8_t y = 0; y < kMatrixHeight; y++) {
    
  uint16_t amount = noise[0][0][y]*4;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);
    
    for(uint8_t x = 1; x < kMatrixWidth; x++) {
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x-1, y)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;
      

      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    } 
    
    PixelA = leds2[XY(0, y)];
    PixelB = leds2[XY(kMatrixWidth-1, y)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
      /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
      */
      

    leds[XY(0, y)] = PixelA + PixelB;
    
  }

}


void MoveFractionalNoiseY2() {
  
  // move delta pixelwise
  for(int x = 0; x < kMatrixWidth; x++) {
    
  uint16_t amount = noise[0][x][0]*4;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);

    for(int y = 0; y < kMatrixWidth-delta; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta)];
    }
    for(int y = kMatrixWidth-delta; y < kMatrixWidth; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta-kMatrixWidth)];
    } 
  }
  //CopyBack();
  
  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for(uint8_t x = 0; x < kMatrixHeight; x++) {
    
  uint16_t amount = noise[0][x][0]*4;
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);
    
    for(uint8_t y = 1; y < kMatrixWidth; y++) {
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x, y-1)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;
      

      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    } 
    
    PixelA = leds2[XY(x, 0)];
    PixelB = leds2[XY(x, kMatrixWidth-1)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
      /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
      */
      

    leds[XY(x, 0)] = PixelA + PixelB;
    
  }

}


void MoveFractionalNoiseX3() {
  
  // move delta pixelwise
  for(int y = 0; y < kMatrixHeight; y++) {
    
  uint16_t amount = noise[0][0][y];
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);

    for(int x = 0; x < kMatrixWidth-delta; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta,y)];
    }
    for(int x = kMatrixWidth-delta; x < kMatrixWidth; x++) {
      leds2[XY(x,y)] = leds[XY(x+delta-kMatrixWidth,y)];
    } 
  }
  //CopyBack();
  
  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for(uint8_t y = 0; y < kMatrixHeight; y++) {
    
  uint16_t amount = noise[0][0][y];
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);
    
    for(uint8_t x = 1; x < kMatrixWidth; x++) {
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x-1, y)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;
      

      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    } 
    
    PixelA = leds2[XY(0, y)];
    PixelB = leds2[XY(kMatrixWidth-1, y)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
      /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
      */
      

    leds[XY(0, y)] = PixelA + PixelB;
    
  }

}


void MoveFractionalNoiseY3() {
  
  // move delta pixelwise
  for(int x = 0; x < kMatrixWidth; x++) {
    
  uint16_t amount = noise[0][x][0];
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);

    for(int y = 0; y < kMatrixWidth-delta; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta)];
    }
    for(int y = kMatrixWidth-delta; y < kMatrixWidth; y++) {
      leds2[XY(x,y)] = leds[XY(x,y+delta-kMatrixWidth)];
    } 
  }
  //CopyBack();
  
  //move fractions
  CRGB PixelA;
  CRGB PixelB;

  //byte fract = beatsin8(60);
  //byte y = 1;

  for(uint8_t x = 0; x < kMatrixHeight; x++) {
    
  uint16_t amount = noise[0][x][0];
  byte delta = 31-(amount/256);
  byte fractions = amount - (delta * 256);
    
    for(uint8_t y = 1; y < kMatrixWidth; y++) {
      PixelA = leds2[XY(x, y)];
      PixelB = leds2[XY(x, y-1)];

      PixelA %= 255 - fractions;
      PixelB %= fractions;
      

      leds[XY(x, y)] = PixelA + PixelB;
      //leds2[XY(x, y)] = 300;
    } 
    
    PixelA = leds2[XY(x, 0)];
    PixelB = leds2[XY(x, kMatrixWidth-1)];

    PixelA %= 255 - fractions;
    PixelB %= fractions;
      /*
      PixelB.r = dim8_raw(PixelB.r);
      PixelB.g = dim8_raw(PixelB.g);
      PixelB.b = dim8_raw(PixelB.b);
      */
      

    leds[XY(x, 0)] = PixelA + PixelB;
    
  }

}

void CLS()  
{
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = 0;
  }
}

void CLS2(){

  for(int y = 0; y < NUM_LEDS; y++) {
    leds2[y] = 0;
  }
}

void MultipleStream() {

  //CLS();
  DimAll(249);

  // gelb im Kreis
  byte xx = 4+sin8( millis() / 10) / 10;
  byte yy = 4+cos8( millis() / 10) / 10;
  leds[XY( xx, yy)] = 0xFFFF00;

  // rot in einer Acht
  xx = 8+sin8( millis() / 46) / 16;
  yy = 8+cos8( millis() / 15) / 16;
  leds[XY( xx, yy)] = 0xFF0000;

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(8);
  MoveFractionalNoiseX();

  MoveY(8);
  MoveFractionalNoiseY();
}


void MultipleStream2() {

  DimAll(230);

  byte xx = 4+sin8( millis() / 9) / 10;
  byte yy = 4+cos8( millis() / 10) / 10;
  leds[XY( xx, yy)] += 0x0000FF;

  xx = 8+sin8( millis() / 10) / 16;
  yy = 8+cos8( millis() / 7) / 16;
  leds[XY( xx, yy)] += 0xFF0000;

  leds[XY( 15,15)] += 0xFFFF00;

  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY2();


  MoveY(3);
  MoveFractionalNoiseX2();
}


void MultipleStream3() {

  //CLS();
  DimAll(235);


  for(uint8_t i = 3; i < 32; i=i+4) {
    leds[XY( i,15)] += CHSV(i*2, 255, 255);
  }

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY2();


  MoveY(3);
  MoveFractionalNoiseX2();
}

void MultipleStream4() {

  //CLS();
  DimAll(235);

  leds[XY( 15, 15)] += CHSV(millis(), 255, 255);


  // Noise
  x[0] += 1000;
  y[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(8);
  MoveFractionalNoiseX();

  MoveY(8);
  MoveFractionalNoiseY();
}


void MultipleStream5() {

  //CLS();
  DimAll(235);


  for(uint8_t i = 3; i < 32; i=i+4) {
    leds[XY( i,31)] += CHSV(i*2, 255, 255);
  }

  // Noise
  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseY2();


  MoveY(4);
  MoveFractionalNoiseX2();
}

void MultipleStream8() {

  //CLS();
  DimAll(230);

  for(uint8_t y = 1; y < 32; y=y+6) {
    for(uint8_t x = 1; x < 32; x=x+6) {

      leds[XY( x, y)] += CHSV((x*y)/4, 255, 255);
    }
  }



  // Noise
  x[0] += 1000;
  y[0] += 1000;
  z[0] += 1000;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);

  MoveX(3);
  MoveFractionalNoiseX2();

  MoveY(3);
  MoveFractionalNoiseY2();

}
