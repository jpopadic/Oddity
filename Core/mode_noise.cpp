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
#include "mode_noise.h"

static Fix16 gStep;
static bool gRed;

// ---------------------------------------------------------------------------------------------------------------------
void NoiseMode::init()
{
  gStep = (fix16_t)gRNG.genUInt32(0, 0xffffff);
  gRed = gRNG.genBool();
}

static byte R[] = { 1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0 };
static byte G[] = { 0, 0, 0, 1, 2, 3, 3, 3, 3, 2, 1, 1, 0 };

// ---------------------------------------------------------------------------------------------------------------------
bool NoiseMode::tick(State &state)
{
  if (state.m_counter > 300)
    return false;

  Fix16 _x, _y, tx, ty, off(0.1f), pr;

  gStep += 0.1f;
  
  off += (gStep.cosM(0.5f) + 1.0f) * 0.1f;

  Fix16 _cos = gStep.cosM(0.25f);
  Fix16 _sin = gStep.sinM(0.25f);

  tx = 0.0f;
  ty = 0.0f;

  _y = ty;
  for (int16_t y=0; y<FRAME_WIDTH; y++)
  {
    for (int16_t x=0; x<FRAME_WIDTH; x++)
    {
      Fix16 __x = (_x * _cos) - (_y * _sin);
      Fix16 __y = (_x * _sin) + (_y * _cos);

      pr = Perlin2(__x + gStep, __y - gStep);

      int index = (pr * fix16_from_float(13.0f)).asInt(); 
      if (index > 0)
      {
        index %= 13;

        setLED(state.m_frame, x, y, gRed ? R[index] : G[index], gRed ? G[index] : R[index]);
      }

      _x += off;
    }

    _x = tx;
    _y += off;
  }

  return true;
}
