/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/

#include "oddity.h"
#include "vfx.h"
#include "stdlib.h"

// ---------------------------------------------------------------------------------------------------------------------
void setLED(pixel* frame, int x, int y, byte r, byte g, bool additive, bool swapXY)
{
  if (swapXY)
  {
    int t = y;
    y = x;
    x = t;
  }

  if(x < 0 || x >= Constants::FrameWidth)
    return;
    
  if(y < 0 || y >= Constants::FrameHeight)
    return;    

#if ROTATE_SCREEN_CCW
  pixel &LEDpixel = frame[x * Constants::FrameWidth + ((Constants::FrameHeight - 1) - y)];
#else
  pixel &LEDpixel = frame[y * Constants::FrameWidth + x];
#endif

  byte red, green;

  if (additive)
  {
    DecodeByte(LEDpixel, red, green);

    red += r;
    green += g;
  }
  else
  {
    red = r;
    green = g;
  }
  
  if (red > 15)
    red = 15;
  if (green > 15)
    green = 15;

  LEDpixel = red | (green << 4);
}


// ---------------------------------------------------------------------------------------------------------------------
Fix16 DistanceBetween(Fix16 x, Fix16 y, Fix16 cX, Fix16 cY)
{
  Fix16 dX = x - cX;
  Fix16 dY = y - cY;
  
  Fix16 distance = (dX * dX) + (dY * dY);
  return distance.sqrt();
}

// ---------------------------------------------------------------------------------------------------------------------
void FullSpectrum(const Fix16& t, byte& r, byte& g)
{
  const byte full_spectrum_red[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 6, 8, 10, 11, 13, 15, 15, 15, 15, 15, 15, 15, 15, 14, 12, 10, 8, 6, 4, 2, 1 }; 
  const byte full_spectrum_green[32] = { 1, 2, 4, 6, 8, 10, 12, 14, 15, 15, 14, 15, 15, 15, 15, 15, 14, 12, 10, 8, 6, 5, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0 }; 

  int index = (t * fix16_from_float(32.0f)).asInt(); 
  index %= 32;

  r = full_spectrum_red[index];
  g = full_spectrum_green[index];
}


// ---------------------------------------------------------------------------------------------------------------------
void ColourGradient(Fix16 t, bool redFirst, bool halfGradient, byte& r, byte& g)
{
  byte A[] = { 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0 };
  byte B[] = { 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 3, 2, 1 };

  int index = (t * fix16_from_float(halfGradient?7.0f:14.0f)).asInt(); 
  index = abs(index);
  index %= 14;
    
  if (redFirst)
  {
    r = A[index];
    g = B[index];
  }
  else
  {
    r = B[index];
    g = A[index];
  }
}

// ---------------------------------------------------------------------------------------------------------------------
void ColourBand(Fix16 t, byte& r, byte& g)
{
  byte A[] = { 0, 0, 1, 2, 4, 3, 0, 0, 0 };
  byte B[] = { 0, 0, 0, 3, 4, 2, 1, 0, 0 };

  int index = (t * fix16_from_float(9.0f)).asInt(); 
  index = abs(index);
  index %= 9;

  r = A[index];
  g = B[index];
}

// ---------------------------------------------------------------------------------------------------------------------
extern const unsigned char* getFontGlyphData16x16(char c);

void draw::FontGlyph16x16(pixel* frame, char c, int16_t fx, int16_t fy, ColourChoice cc)
{
  const unsigned char* fontBuf = getFontGlyphData16x16(c);
  if (!fontBuf)
    return;

  byte r, g;
  int idx = 0;
  for (int y=0; y<16; y++)
  {
    for (int x=0; x<16; x++)
    {
      int16_t colInd = fontBuf[idx] != 0 ? (fontBuf[idx] == 1 ? 2 : 15) : 0;
      GetBasicColourIdx(colInd, cc, r, g);
      setLED(frame, x + fx, y + fy, r, g);
      idx ++;
    }
  }
}

// ---------------------------------------------------------------------------------------------------------------------
extern const unsigned char* getFontGlyphData8x8(char c);

void draw::FontGlyph8x8(pixel* frame, char c, int16_t fx, int16_t fy, ColourChoice cc)
{
  const unsigned char* fontBuf = getFontGlyphData8x8(c);
  if (!fontBuf)
    return;

  byte r, g;
  for (int x=0; x<8; x++)
  {
    for (int y=0; y<8; y++)
    {
      int set = fontBuf[y] & 1 << x;

      if (set > 0)
      {
        GetBasicColourIdx(set * 15, cc, r, g);
        setLED(frame, x + fx, y + fy, r, g);
      }
    }
  }
}