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
#include "lookup_flame.h"

struct EffectData
{
  enum
  {
    CoalWidth = Constants::FrameWidth - 2,
    CoalHeight = 3,
    Width = Constants::FrameWidth,
    Height = Constants::FrameHeight,
    BufferHeight = Constants::FrameHeight + CoalHeight,
    BufferSize = (Width * BufferHeight)
  };

  uint8_t buffer[BufferSize * 2];
  int frontBuffer;
  int backBuffer;

  int time;

  Fix16 noiseTimeScale;
  Fix16 noiseXScale;
  Fix16 decay;
};

uint8_t* frontBuffer;
uint8_t* backBuffer;

#ifdef _MSC_VER
static_assert(sizeof(EffectData) < Constants::MemoryPool, "EffectData too big for global memory pool");
#endif // _MSC_VER


// ---------------------------------------------------------------------------------------------------------------------
void flame_init(FXState& state)
{
  EffectData* data = (EffectData*)state.store;

  for(int i = 0; i < EffectData::BufferSize * 2; ++i)
  {
    data->buffer[i] = 0;
  }

  backBuffer = data->buffer;
  frontBuffer = &data->buffer[EffectData::BufferSize];

  data->time = 0;

  data->noiseTimeScale = 0.1f;
  data->noiseXScale = 0.5f;
  data->decay = 8.3f;
}

// ---------------------------------------------------------------------------------------------------------------------
bool flame_tick(const FrameInput& input, FrameOutput& output, FXState& state)
{
  EffectData* data = (EffectData*)state.store;

  // Inputs.
  //
  if(input.dialChange[1] != 0)
  {
    data->decay += (input.dialChange[1] * 0.05f);
  }

  // Coals.
  for(int y = EffectData::BufferHeight - EffectData::CoalHeight - 1; y < EffectData::BufferHeight; ++y)
  {  
    for(int x = (EffectData::Width - EffectData::CoalWidth) / 2 ; x < EffectData::Width - (EffectData::Width - EffectData::CoalWidth) / 2; ++x)
    { 
      Fix16 noise = Perlin3(x * data->noiseXScale, y * 0.5f, data->time * data->noiseTimeScale) + 0.5f;
      int coalValue = fix16_to_int(noise * 400.0f);
      backBuffer[y * EffectData::Width + x] = (coalValue < 255 ? coalValue : 255);
    }
  }
  data->time++;

  // Flame.
  //
  uint8_t p1, p2, p3, p4, p5, p6, p7;
  int u, d, l, r;

  for(int x = 0; x < EffectData::Width; ++x)
  {
    l = x == 0 ? 0 : x - 1;
    r = x == EffectData::Width - 1 ? EffectData::Width - 1 : x + 1;

    for(int y = 1; y < EffectData::BufferHeight - 1; ++y)
    { 
      u = y == 0 ? 0 : y - 1;
      d = y == EffectData::BufferHeight - 1 ? EffectData::BufferHeight : y + 1;

      // p1 dest p5
      // p2 src  p6
      // p3 p4   p7

      p1 = backBuffer[u * EffectData::Width + l ];
      p2 = backBuffer[y * EffectData::Width + l ];
      p3 = backBuffer[d * EffectData::Width + l ];

      p4 = backBuffer[d * EffectData::Width + x ];

      p5 = backBuffer[u * EffectData::Width + r ];
      p6 = backBuffer[y * EffectData::Width + r ];
      p7 = backBuffer[d * EffectData::Width + r ];

      int src = backBuffer[y * EffectData::Width + x ];

      int sum = src + p1 + p2 + p3 + p4 + p5 + p6 + p7;
      Fix16 result = fix16_from_int(sum);

      result /= data->decay; //8.3f;

      const Fix16 maxValue((int16_t)255);
      result = (result < maxValue ? result : maxValue);
      frontBuffer[(y - 1) * EffectData::Width + x] = fix16_to_int(result);
    }
  }

  // Cooling.
  //
  for(int i = 0; i < EffectData::Width * EffectData::Height; ++i)
  {
    int adjusted = (int)frontBuffer[i] - (int)coolingMap[i] / 4;
    frontBuffer[i] = adjusted > 0 ? adjusted : 0;
  }

  // Draw.
  for(int x = 0; x < EffectData::Width; ++x)
  {
    for(int y = 0; y < EffectData::Height; ++y)
    { 
      int index = frontBuffer[y * EffectData::Width + x];
      index = index >> 3;  
      setLED(output.frame, x, y, Rr[index], Gr[index]);
    }
  }

  // Flip buffer.
  uint8_t* front = frontBuffer;
  frontBuffer = backBuffer;
  backBuffer = front;

  return true;
}
