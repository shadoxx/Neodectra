/******************************************************************************
* Neodectra - Unofficial compatible firmware for use with Audectra
*             [http://www.audectra.com/]
*
* Firmware written by Brandon Wooldridge [brandon+neodectra@hive13.org]
* Audectra written by A1i3n37 [support@audectra.com]
*
* This program is provided as is. People with epilepsy should exercise extreme
*   caution when using this firmware. All rights to Audectra are retained by
*   A1i3n37 and his team. Neodectra is an independent project, and does not
*   represent or claim to represent Audectra in any way, shape, or form.
*
* FILE: Neodectra.ino
* DESC: Our main codebase for the Teensy
*
* TODO:
*  - introduce support for HSV instead of RGB. intensity of color depends on
*    range of value
*  - Implement latest version of FastLED (version 3.1 as of writing)
*    
******************************************************************************/
// FIRMWARE VERSION

// INCLUDES
#include "FastLED.h"
#include "config.h"
#include "Neodectra.h"

// FUNCTION PROTOTYPES
void fadeAfterDelay( DelayTimer *dTimer, uint8_t fDelay, uint8_t fPercentage );
void colorSetSquare( RGBValue rgbPixel );

// GLOBAL VARIABLES
CRGB ledStrip[STRIP_LENGTH];
DelayTimer UpdateDelay, LFODelay, FadeDelay, VUDelay;
uint8_t Offset = 0;
uint16_t cursorPixel = 0;
boolean Increase, HasIdentified, fadeDelayEnabled;

void setup() {
  LEDS.setBrightness(BRIGHTNESS);  // between 0 and 255, default 25% brightness (64)
  LEDS.addLeds<CHIPSET, DATAPIN, PIXEL_ORDER>(ledStrip, STRIP_LENGTH);  // initialize our LED object
  memset(ledStrip, 0,  STRIP_LENGTH * sizeof(struct CRGB));  // set all LEDs to off

  pinMode(DATAPIN, OUTPUT);  // setup serial interface for communication
  pinMode(17,      INPUT);   // input for rng
  pinMode(5,       INPUT);   // input for rng

  Serial.begin(BAUDRATE);

  // flash our LEDS to let us know initialization succeeded
  memset(ledStrip, 100, STRIP_LENGTH * sizeof(struct CRGB));

  UpdateDelay.prevTime = 0;
  LFODelay.prevTime = 0;
  HasIdentified = false;
  fadeDelayEnabled = true;

  // seed the rng for static function
  randomSeed( analogRead(17) ^ analogRead(5) );

  FastLED.show();  // update our strip, to blank out everything
  delay(500);      // let the teensy get to a ready state
}

void loop() {
  UpdateDelay.currTime = LFODelay.currTime = FadeDelay.currTime = VUDelay.currTime = millis();

  if( Serial.available() >= 4 ) {
    char buffer[4] = { '\0' };
    Serial.readBytes(buffer,4);  // read in our color values from the Audectra client
    if( buffer[0] == 1 && HasIdentified == false ) fwIdentify();  // identify ourselves as the proper COM device

    if( UpdateDelay.currTime - UpdateDelay.prevTime > SAMPLERATE ) {
      UpdateDelay.prevTime = UpdateDelay.currTime;
      
      RGBValue thisPixel;
      thisPixel.Red    = buffer[0+AUDECTRA_VERSION];
      thisPixel.Green  = buffer[1+AUDECTRA_VERSION];
      thisPixel.Blue   = buffer[2+AUDECTRA_VERSION];
      
      switch( CURRENT_EFFECT ) {
      //switch( random(4,6) ) {
        case 0:
          colorSetAll( buffer[0+AUDECTRA_VERSION], buffer[1+AUDECTRA_VERSION], buffer[2+AUDECTRA_VERSION] );
          break;
        case 1:
          colorSetSplit( buffer[0+AUDECTRA_VERSION], buffer[1+AUDECTRA_VERSION], buffer[2+AUDECTRA_VERSION], &Offset );
          break;
        case 2:
          colorSetVU( buffer[0+AUDECTRA_VERSION], buffer[1+AUDECTRA_VERSION], buffer[2+AUDECTRA_VERSION], MASTER_GAIN );
          break;
        case 3:
          colorSetVUSplit( buffer[0+AUDECTRA_VERSION], buffer[1+AUDECTRA_VERSION], buffer[2+AUDECTRA_VERSION] );
          break;
        case 4:
          // trying something funky
          ( thisPixel.Red > 120 ) ? colorSetSplit( thisPixel.Red, thisPixel.Green, thisPixel.Blue, &Offset ) : colorSetSquare( &thisPixel );
          mirrorDisplay();
          break;
        case 5:
            ( thisPixel.Red > 120 ) ? colorSetSplit( (thisPixel.Red ^ thisPixel.Green ^ thisPixel.Blue), thisPixel.Green, thisPixel.Blue, &Offset ) : drawInner( &thisPixel );
            mirrorDisplay();
            if( VUDelay.currTime - VUDelay.prevTime > VU_DELAY ) {
              colorSetNoise( &thisPixel );
            }
            break;
        case 6:
            //ledStrip[linearCalc(0, 0, 16, 16)] = CRGB(thisPixel.Red, thisPixel.Green, thisPixel.Blue );
            if( VUDelay.currTime - VUDelay.prevTime > VU_DELAY ) {
              drawOuter(&thisPixel);
              drawMiddle(&thisPixel);
              colorSetNoise( &thisPixel );
            }
            
            //line( random( 0, 3), random( 0, 3), random(12,16), random(12,16), &thisPixel);
            //line( random( 0, 3), random(12,16), random(12,16), random( 0, 3), &thisPixel);
            //line( random( 0, 2), random( 0, 2), random(14,16), random(14,16), &thisPixel);
            //line( random( 0, 2), random(14,16), random(14,16), random( 0, 2), &thisPixel);
            line( 7, 0, 7, 16, &thisPixel);
            line( 8, 0, 8, 16, &thisPixel);
            //line( 0, 7, 16, 7, &thisPixel);
            //line( 0, 8, 16, 8, &thisPixel);
            
            mirrorDisplay();
            break;
      }

      if( fadeDelayEnabled == true && (LFODelay.currTime - LFODelay.prevTime > LFO_RATE) ) {
        LFODelay.prevTime = LFODelay.currTime;
        lfoCalc(2, &Offset, &Increase);
      }
    }

    // from lib8tion.h
    //uint8_t bright = beatsin8( 128 /*BPM*/, 180 /*dimmest*/, 220 /*brightest*/ );
    //FastLED.setBrightness( bright );
    
    FastLED.show();
  }
  else {
    // if we've received no data from the serial port, start fading display until all pixels are 0, or we receive data
    if( ((FadeDelay.currTime - FadeDelay.prevTime) > FADE_DELAY) && (FADE_DELAY != 0) ) {
      FadeDelay.prevTime = FadeDelay.currTime;
      for( int pixel = 0; pixel < STRIP_LENGTH; pixel++ ) {
        ledStrip[pixel].fadeToBlackBy( FADE_PERCENT );
      }
      FastLED.show();
    }
  }
}

// VISUAL EFFECTS

// set all LEDs in a strip to the same color
void colorSetAll(uint8_t Red, uint8_t Green, uint8_t Blue) {
  for( int i = 0; i < STRIP_LENGTH; i++ ) ledStrip[i] = CRGB( Red, Green, Blue );
}

void colorSetSquare( RGBValue *rgbPixel )
{
  drawOuter( rgbPixel );
  drawInner( rgbPixel );
  drawMiddle( rgbPixel);
}


// split the strip into pixel groups of three and update red, green, and blue as determined by switch function
void colorSetSplit(uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t* DelayOffset) {
  for( int i = 0; i < STRIP_LENGTH; i++ ) {
    int currentPixel = (i + *DelayOffset) % STRIP_LENGTH;

    switch ( i % 3 ) {
    case 0:
      ledStrip[currentPixel] = CRGB( Red, 0, 0 );
      break;
    case 2:
      ledStrip[currentPixel] = CRGB( 0, Green, 0 );
      break;
    case 1:
      ledStrip[currentPixel] = CRGB( 0, 0, Blue );
      break;
    }
  }
}

// Audectra Settings: Bass[85,8] - Mid[65,16] - High[40,32], ymmv
void colorSetVU(uint8_t Red, uint8_t Green, uint8_t Blue, uint8_t vuGain) {
  unsigned int maxBounds = map(volCalc(&Red, &Green, &Blue, vuGain), 0, (255*3), 0, STRIP_LENGTH);
  unsigned int newBounds = map(volCalc(&Red, &Green, &Blue, vuGain), 0, (255*3), 0, STRIP_LENGTH);
  
  (abs(newBounds - maxBounds) > STRIP_LENGTH/32) ? maxBounds = newBounds : maxBounds -= 5;
  
  if( VUDelay.currTime - VUDelay.prevTime > VU_DELAY ) {
    for( int i = 0; i <= STRIP_LENGTH; i++ ) {
      ledStrip[i] = CRGB( Red, Green, Blue );
      if (i > maxBounds) ledStrip[i] = CRGB( 0, 0, 0 );
    }
  }
}

// VU meter effect, shoots from the middle outward in both directions
void colorSetVUSplit(uint8_t Red, uint8_t Green, uint8_t Blue) {
  unsigned int maxBounds = map( volCalc(&Red,&Green,&Blue,MASTER_GAIN), 0, readPot(), 0, STRIP_CENTER );

  maxBounds = constrain(maxBounds, 0, STRIP_CENTER);  // may not be needed but leaving for now

  // calculate the pixel we're on, starting from the center. set right half of strip to proper values. mirror to left side of strip
  // NOTE: does not handle an odd number of pixels, will have an off by one error
  for( int i = 0; i < STRIP_CENTER; i++ ) {
    ledStrip[STRIP_CENTER + i] = CRGB( Red, Green, Blue );  // right
    if( i > maxBounds ) ledStrip[STRIP_CENTER + i] = CRGB( 0, 0, 0 );
    ledStrip[(STRIP_CENTER - 1) - i] = ledStrip[STRIP_CENTER + i];  // left
  }
}

void colorSetNoise( RGBValue *rgbPixel ) {
    //lfoCalc( 255, &Offset, &Increase ); // change the hue every so often
    
    //for( int i = 0; i <= INTENSITY; i++ )
    //{
    // https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
     
      uint8_t RndHue   = getHueFromRGB( rgbPixel->Red, rgbPixel->Green, rgbPixel->Blue );
      //uint8_t RndSat   = random(50,150);
      uint8_t RndSat   = random(50,200);
      uint8_t RndVib   = random(100,200);
      
      for( int f = 0; f <= 4; f++ ) {
        ledStrip[random(STRIP_LENGTH)] = CHSV( RndHue, RndSat, RndVib );
      }
    //} 
}

// UTILITY FUNCTIONS
void fwIdentify() {
  HasIdentified = true;
  memset(ledStrip, 50, STRIP_LENGTH * sizeof(struct CRGB));
  Serial.println("DIYAudectra"); // ID
  Serial.println("1.1.0\n");  // Firmware Version
}

unsigned int readPot() {
  unsigned int readVal = map(analogRead(POTPIN), 0, 1023, 0, MAX_VOLUME_RANGE);
  if( !POT_READ ) readVal = 256;  // magic value that seems to work best
  return readVal;

  //return map(analogRead(POTPIN), 0, 1023, 0, MAX_VOLUME_RANGE);
}

unsigned int volCalc(uint8_t* freqHi, uint8_t* freqMid, uint8_t* freqLow, uint8_t MasterGain) {
  // return a "Volume" level by combining all of our color codes, then multiplying them by a gain
  return ( (((int)freqHi * HI_GAIN) + ((int)freqMid * MID_GAIN) + ((int)freqLow * LOW_GAIN)) / 3) * MasterGain;
}

// lfoCalc: counts up to lfoMax, then starts counting down, then does it again
void lfoCalc(uint8_t lfoMax, uint8_t* OffsetVal, boolean* IncSwitch) {
  // check our switch, then increment or decrement our value as needed
  (*IncSwitch) ? *OffsetVal +=1 : *OffsetVal -=1;
	
  // figure out if we should start increasing or decreasing
  if (*OffsetVal == 0) *IncSwitch = true;
  if (*OffsetVal == lfoMax) *IncSwitch = false;
}

void fadeAfterDelay( DelayTimer *dTimer, uint8_t fDelay, uint8_t fPercentage ) {
  if( ((dTimer->currTime - dTimer->prevTime) > fDelay) && (fDelay != 0) ) {
    dTimer->prevTime = dTimer->currTime;
    for( int pixel = 0; pixel < STRIP_LENGTH; pixel++ ) {
      ledStrip[pixel].fadeToBlackBy( fPercentage );
    }
  }
}

// treat an arragement of LEDS in a zig zig as continuous rows
// Math credit to Chris Hodapp (@hodapp87)
unsigned int linearCalc( unsigned int x, unsigned int y, unsigned int x_len, unsigned int y_len )
{
  return x_len * x + y + ((x_len - 1) - (y % x_len)*2) * (x % 2);
}

// For tiled Grids of LEDs
unsigned int xyCalc( unsigned int x, unsigned int y, unsigned int x_len, unsigned int y_len )
{
    uint8_t j = x / x_len;

    uint8_t xj = x - j * x_len;
    uint8_t yj = y;

    return j * (x_len * y_len) + linearCalc( xj, yj, x_len, y_len );
}

void drawSquare( int x_size, int y_size, int x_offset, int y_offset, RGBValue *rgbPixel ) {
  for( int y = y_offset; y < (y_size + y_offset); y++ ) {
    for( int x = x_offset; x < (x_size + x_offset); x++ ) {
      ledStrip[linearCalc(x, y, 16, 16)] = CRGB( rgbPixel->Red, rgbPixel->Green, rgbPixel->Blue );
    }
  }
}

void drawOuter( RGBValue *rgbPixel )
{ // outer ring is Red because we're subtracting red and blue values
  RGBValue thisPixel = *rgbPixel;
  
  thisPixel.Green = 0;
  thisPixel.Blue = 0;
  
  drawSquare( 4, 16,  0,  0, &thisPixel);
  drawSquare( 8,  4,  4,  0, &thisPixel);
  drawSquare( 8,  4,  4, 12, &thisPixel);
  drawSquare( 4, 16, 12,  0, &thisPixel);
}

void drawMiddle( RGBValue *rgbPixel )
{
  if( rgbPixel->Blue > 0 ) {
    RGBValue thisPixel = *rgbPixel;
    thisPixel.Green = 0;
    thisPixel.Red = 0;
    drawSquare( 6, 6, 5, 5, &thisPixel );
  } else {
    drawSquare( 6, 6, 5, 5, rgbPixel );
  }
}

void drawInner( RGBValue *rgbPixel )
{
  if( rgbPixel->Red > 0 ) {
    drawSquare( 12,  12,  2,  2, rgbPixel);
  }
  else {
    drawSquare( 16,  16,  0,  0, rgbPixel);
  }
}

// returns the hue of an RGB object
// slightly modified from a java example on StackOverflow. I didn't save the link I'm sorry!
unsigned int getHueFromRGB(int red, int green, int blue) {
    float min = min(min(red, green), blue);
    float max = max(max(red, green), blue);

    float hue = 0x0f;
    
    if (max == red) {
        hue = (green - blue) / (max - min);
    } else if (max == green) {
        hue = 0x2f + (blue - red) / (max - min);
    } else {
        hue = 0x4f + (red - green) / (max - min);
    }

    hue = hue * 60;
    if (hue < 0) hue = hue + 360;

    return round(hue);
}

// this is just a quick function to copy the contents of the left panel over to the right panel
// of a 16x16 grid. messy, and hopefully won't be here for long. should really be done with
// the memset() function, but that's really hard and i don't feel like doing that right now.
// brandon - 10/16/2014
void mirrorDisplay()
{
  for( int i = 0; i < 256; i++ )
  {
    ledStrip[i+256] = ledStrip[i];  // for the second panel in sequence
    ledStrip[i+512] = ledStrip[i];  // for the third panel in sequence
  }
}

// https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
// This algorithm draws a line. You can't get simpler than a line. :D
void line(int x0, int y0, int x1, int y1, RGBValue *rgbPixel) {
 
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
    ledStrip[linearCalc(x0, y0, 16, 16)] = CRGB(rgbPixel->Red, rgbPixel->Green, rgbPixel->Blue );
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}
