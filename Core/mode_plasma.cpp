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
#include "mode_plasma.h"

static Fix16 gStep;
static bool gMutations[16];
static Fix16 gAttractorScales[4];

// ---------------------------------------------------------------------------------------------------------------------
void PlasmaMode::init()
{
  gStep = (fix16_t)gRNG.genUInt32(0, 0xffffff);

  for (int i=0; i<16; i++)
    gMutations[i] = gRNG.genBool();

  for (int i=0; i<4; i++)
    gAttractorScales[i] = (fix16_t)gRNG.genUInt32(0, 0xfffff);
}

// ---------------------------------------------------------------------------------------------------------------------
bool PlasmaMode::tick(State &state)
{
  if (state.m_counter > 300)
    return false;

  byte r, g;
  Fix16 mid(8.0f);

  Fix16 tx1, ty1, tx2, ty2;

  Fix16 XM1 = gStep.sinMV(0.3f, 3.0f) * gAttractorScales[0];
  Fix16 YM1 = gStep.sinMV(0.1f, 2.0f) * gAttractorScales[1];
  Fix16 XM2 = gStep.cosMV(0.2f, -3.0f) * gAttractorScales[2];
  Fix16 YM2 = gStep.cosMV(0.05f, -5.0f) * gAttractorScales[3];

  tx1 = mid + (gStep.sinV(-0.25f) * XM1);
  ty1 = mid + (gStep.cosV(0.1f) * YM1);
  tx1 = mid + (gStep.sinMV(-0.5f, 4.0f) * XM2);
  ty1 = mid + (gStep.cosV(0.1f) * YM2);

  for (int y=0; y<FRAME_HEIGHT; y++)
  {
    for (int x=0; x<FRAME_WIDTH; x++)
    {
      Fix16 fx((float)x), fy((float)y);

      fx += fx.sinMV(0.3f, YM1);
      fy += fy.cosMV(0.3f, XM2);

      Fix16 dist1 = DistanceBetween(fx, fy, tx1, ty1);
      Fix16 dist2 = DistanceBetween(fx, fy, tx2, ty2);

      Fix16 tn = Fix16::atan2(dist1, dist2) * ((gStep.cosM(0.25f) + 2.0f) * 0.8f);

      tn += gStep * 0.4f;

      tn += fx.cosMV(0.1f, YM2) * (gStep.sinMV(0.1f, 25.0f) * 0.3f);
      tn -= fy.sinMV(0.1f, XM1) * (gStep.cosMV(0.1f, 15.0f) * 0.3f);

      ColourGradient(tn, gMutations[0], gMutations[1], r, g);
      setLED(state.m_frame, x, y, r, g);
    }
  }

  gStep += 0.1f;
  return true;
}
