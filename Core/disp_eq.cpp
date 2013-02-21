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

struct EffectData
{

};

#ifdef _MSC_VER
static_assert(sizeof(EffectData) < Constants::MemoryPool, "EffectData too big for global memory pool");
#endif // _MSC_VER

// ---------------------------------------------------------------------------------------------------------------------
void eq_init(FXState& state)
{
}


// ---------------------------------------------------------------------------------------------------------------------
bool eq_tick(FrameOutput& output, FXState& state)
{
  Fix16 pT = fix16_mul(fix16_from_int(state.counter), fix16_from_float(0.1f));
  Fix16 pY = fix16_mul(fix16_from_int(state.counter), fix16_from_float(0.03f));

  for (int32_t x=0; x<Constants::FrameWidth; x++)
  {
    Fix16 pX = fix16_mul(fix16_from_int(x), fix16_from_float(0.3f));
    pX += pT;

    Fix16 row = Perlin2(pX, pY);
    row *= 17.0f;

    byte A[] = { 1, 3, 8, 15, 13,  7,  4,  1, 0 };
    byte B[] = { 0, 0, 0,  1,  3,  6, 13, 15, 7 };

    setLED(output.frame, x, 8, 1, 0);

    int rowInt = row.asInt();
    if (rowInt < 0)
      rowInt = -rowInt;
    if (rowInt > 8)
      rowInt = 8;

    for (int32_t y=0; y<rowInt; y++)
    {
      setLED(output.frame, x, 8 - y, A[y], B[y]);
      setLED(output.frame, x, 8 + y, A[y], B[y]);
    }
  }

  return true;
}