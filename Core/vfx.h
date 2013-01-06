/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/

#pragma once

#include "oddity.h"
#include "fxp3d.h"

#define oMIN(a,b)	(((a)<(b))?(a):(b))	
#define oMAX(a,b)	(((a)>(b))?(a):(b))	

Fix16 DistanceBetween(Fix16 x, Fix16 y, Fix16 cX, Fix16 cY);
void ColourGradient(Fix16 t, bool redFirst, bool halfGradient, byte& r, byte& g);

// ---------------------------------------------------------------------------------------------------------------------
Fix16 Perlin3(Fix16 x, Fix16 y, Fix16 z);
Fix16 Perlin2(Fix16 x, Fix16 y);

// ---------------------------------------------------------------------------------------------------------------------
// helpers to decode the R/G intensities out of a single pixel byte
inline void DecodeByte(byte pixel, byte &r, byte &g)
{
  r = pixel & 0x0f;
  g = (pixel >> 4) & 0x0f;
}

inline void DecodeByteAdditive(byte pixel, byte &r, byte &g)
{
  r += pixel & 0x0f;
  g += (pixel >> 4) & 0x0f;
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename _T>
inline void ZeroFrame(_T* frame)
{
  for(int i = 0; i < Constants::FrameSize; ++i)
    frame[i] = 0;
}

template <typename _Tf, typename _Tt>
inline void CopyFrame(_Tf* from, _Tt* to)
{
  for(int i = 0; i < Constants::FrameSize; ++i)
    to[i] = from[i];
}

// ---------------------------------------------------------------------------------------------------------------------
// the long-form way to set a single LED value, with overflow protection and so on
void setLED(byte* frame, int x, int y, byte r, byte g, bool additive = false, bool swapXY = false);


// ---------------------------------------------------------------------------------------------------------------------
// a simple 0.0 < t < 1.0 colour generator
enum ColourChoice
{
  Red,
  Green,
  Lime,
  Orange,
  Yellow
};

inline void GetBasicColour(int16_t index, ColourChoice cc, byte& r, byte& g)
{
  if (index > 3)
    index = 3;
  if (index < 0)
    index = 0;

  switch (cc)
  {
  case Red:
    {
      const byte R[] = { 0, 1, 2, 3 };
      const byte G[] = { 0, 0, 0, 0 };

      r = R[index];
      g = G[index];
    }
    break;
  case Green:
    {
      const byte R[] = { 0, 0, 0, 0 };
      const byte G[] = { 0, 1, 2, 3 };

      r = R[index];
      g = G[index];
    }
    break;
  case Lime:
    {
      const byte R[] = { 0, 1, 1, 2 };
      const byte G[] = { 0, 1, 2, 3 };

      r = R[index];
      g = G[index];
    }
    break;
  case Orange:
    {
      const byte R[] = { 0, 1, 2, 3 };
      const byte G[] = { 0, 0, 0, 3 };

      r = R[index];
      g = G[index];
    }
    break;
  case Yellow:
    {
      const byte R[] = { 0, 1, 2, 3 };
      const byte G[] = { 0, 1, 2, 3 };

      r = R[index];
      g = G[index];
    }
    break;
  }
}

inline void GetBasicColour(Fix16 t, ColourChoice cc, byte& r, byte& g)
{
  int16_t index = (t * fix16_from_float(4.0f)).asInt();
  GetBasicColour(index, cc, r, g);
}

// ---------------------------------------------------------------------------------------------------------------------
// rendering tools
namespace draw
{
  
  // antialiased line
  void WuLine(byte* frame, Fix16 x1, Fix16 y1, Fix16 x2, Fix16 y2, ColourChoice cc);


  void FontGlyph16x16(byte* frame, char c, int16_t fx, int16_t fy, ColourChoice cc);
  void FontGlyph8x8(byte* frame, char c, int16_t fx, int16_t fy, ColourChoice cc);

} // namespace draw

// ---------------------------------------------------------------------------------------------------------------------
class TextScroller
{
public:
  TextScroller();
  
  void Init(const char* text);

  bool Render(byte* frame, ColourChoice cc);

protected:
  const char* m_text;

  Fix16   m_wave;
  int32_t m_len,
          m_scroll, 
          m_char, 
          m_nextPixelCountdown;
};