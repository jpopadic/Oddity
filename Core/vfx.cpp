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

// ---------------------------------------------------------------------------------------------------------------------
void setLED(byte* frame, int x, int y, byte r, byte g, bool additive, bool swapXY)
{
  if (swapXY)
  {
    int t = y;
    y = x;
    x = t;
  }

  if(x < 0 || x >= FRAME_WIDTH)
    return;
    
  if(y < 0 || y >= FRAME_HEIGHT)
    return;    

  byte &pixel = frame[y * FRAME_WIDTH + x];
  byte red, green;
  DecodeByte(pixel, red, green);


  red += r;
  green += g;


  if (red > 3)
    red = 3;
  if (green > 3)
    green = 3;

  pixel = red | (green << 4);
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
void ColourGradient(Fix16 t, bool redFirst, bool halfGradient, byte& r, byte& g)
{
  byte A[] = { 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0 };
  byte B[] = { 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 1 };

  int index = (t * fix16_from_float(halfGradient?6.0f:12.0f)).asInt(); 
  index = abs(index);
  index %= 12;
    
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
extern const unsigned char* getFontGlyphData(char c);

void draw::FontGlyph(byte* frame, char c, int16_t fx, int16_t fy, ColourChoice cc)
{
  const unsigned char* fontBuf = getFontGlyphData(c);
  if (!fontBuf)
    return;

  byte r, g;
  int idx = 0;
  for (int y=0; y<FRAME_HEIGHT; y++)
  {
    for (int x=0; x<FRAME_WIDTH; x++)
    {
      GetBasicColour((int16_t)fontBuf[idx], cc, r, g);
      setLED(frame, x + fx, y + fy, r, g);
      idx ++;
    }
  }
}