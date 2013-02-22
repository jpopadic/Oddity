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
  Fix16 glowBuffer[Constants::FrameWidth * (Constants::FrameHeight + 1)];
  Fix16 fdecay;
  bool red;
  int16_t tickwait;
};

#ifdef _MSC_VER
static_assert(sizeof(EffectData) < Constants::MemoryPool, "EffectData too big for global memory pool");
#endif // _MSC_VER

// ---------------------------------------------------------------------------------------------------------------------
void fire_init(FXState& state)
{
  EffectData* data = (EffectData*)state.store;

  Fix16 fzero(fix16_zero);
  ZeroFrame(data->glowBuffer, fzero);
  data->fdecay = 0.20f;
  data->red =  state.rng.genBool();
  data->tickwait = 0;
}


// ---------------------------------------------------------------------------------------------------------------------
bool fire_tick(const FrameInput& input, FrameOutput& output, FXState& state)
{
  EffectData* data = (EffectData*)state.store;

  data->tickwait --;
  if (data->tickwait > 0)
    return true;
  data->tickwait = 3;

  Fix16 dial2((int16_t)input.dialChange[2]);
  dial2 *= fix16_from_float(0.005f);
  data->fdecay += dial2;

  output.clear();

  const byte fire_red[32]   = {  0,  0,  0,  1,  1,  2,  2,  2,  3,  3,  4,  4,  5,  5,  6,  7,  8,  8,  9, 10, 11, 11, 12, 13, 14, 14, 15, 15, 15, 15, 15, 15 }; 
  const byte fire_green[32] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  5,  8, 11, 14, 15 }; 

  for (int y=0; y<Constants::FrameHeight; y++)
  {
    for (int x=0; x<Constants::FrameWidth; x++)
    {
      Fix16 pix = fix16_zero;

      Fix16 pixelA = data->glowBuffer[(y+1) * Constants::FrameWidth + x];

      Fix16 *pixelB = (x > 0) ? &data->glowBuffer[(y+1) * Constants::FrameWidth + (x - 1)] : 0;
      Fix16 *pixelC = (x < Constants::FrameWidth - 1) ? &data->glowBuffer[(y+1) * Constants::FrameWidth + (x + 1)] : 0;

      Fix16 *pixelD = (x > 0) ? &data->glowBuffer[y * Constants::FrameWidth + (x - 1)] : 0;
      Fix16 *pixelE = (x < Constants::FrameWidth - 1) ? &data->glowBuffer[y * Constants::FrameWidth + (x + 1)] : 0;

      pix += pixelA;

      if (pixelB)
        pix += *pixelB;
      if (pixelC)
        pix += *pixelC;
      if (pixelD)
        pix += *pixelD;
      if (pixelE)
        pix += *pixelE;

      pix *= data->fdecay;


      data->glowBuffer[ (y * Constants::FrameHeight) + x ] = pix;

      Fix16 pxc(pix);
      pxc *= fix16_from_float(31.0f);
      int16_t index = pxc.asInt();

      if (index > 31)
        index = 31;
      if (index < 0)
        index = 0;

      setLED(output.frame, x, y, fire_red[index], fire_green[index]);
    }
  }

  for (int i=0; i<4; i++)
  {
    Fix16 mulp(0.05f);
    mulp *= fix16_from_int((int16_t)state.rng.genInt32(1, 25));

    data->glowBuffer[ (16 * Constants::FrameHeight) + (state.rng.genUInt32(0, 15)) ] = mulp;
  }

  for (int i=0; i<2; i++)
    data->glowBuffer[ (state.rng.genUInt32(0, 15) * Constants::FrameHeight) + (state.rng.genUInt32(0, 15)) ] = fix16_zero;


  return true;
}