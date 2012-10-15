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
#include "mode_flame.h"


static int8_t gGlowBuffer[FRAME_WIDTH * (FRAME_HEIGHT + 1)];
static Fix16 gDecay;
static bool gRed;

// ---------------------------------------------------------------------------------------------------------------------
void FlameMode::init()
{
  ZeroFrame(gGlowBuffer);
  gDecay = 0.223f;
  gRed =  gRNG.genBool();
}

// ---------------------------------------------------------------------------------------------------------------------
bool FlameMode::tick(State &state)
{
  if (state.m_counter > 300)
    return false;

  if (state.m_inputs.encoderChange[0])
    gRed = !gRed;

  const byte Rr[] = { 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3 };
  const byte Gr[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3 };

  const byte Rg[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3 };
  const byte Gg[] = { 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3 };

  for (int y=0; y<FRAME_HEIGHT; y++)
  {
    for (int x=0; x<FRAME_WIDTH; x++)
    {
      int8_t pix = 0;

      int8_t pixelA = gGlowBuffer[(y+1) * FRAME_WIDTH + x];

      int8_t *pixelB = (x > 0) ? &gGlowBuffer[(y+1) * FRAME_WIDTH + (x - 1)] : 0;
      int8_t *pixelC = (x < FRAME_WIDTH - 1) ? &gGlowBuffer[(y+1) * FRAME_WIDTH + (x + 1)] : 0;

      int8_t *pixelD = (x > 0) ? &gGlowBuffer[y * FRAME_WIDTH + (x - 1)] : 0;
      int8_t *pixelE = (x < FRAME_WIDTH - 1) ? &gGlowBuffer[y * FRAME_WIDTH + (x + 1)] : 0;

      pix += pixelA;

      if (pixelB)
        pix += *pixelB;
      if (pixelC)
        pix += *pixelC;
      if (pixelD)
        pix += *pixelD;
      if (pixelE)
        pix += *pixelE;

      Fix16 fPix((int16_t)pix);
      fPix *= gDecay;
      pix = fPix.asInt();
      pix -= 1;

      if (pix < 0)
        pix = 0;
      if (pix > 12)
        pix = 12;


      gGlowBuffer[ (y * FRAME_HEIGHT) + x ] = pix;

      setLED(state.m_frame, x, y, gRed ? Rr[pix] : Rg[pix], gRed ? Gr[pix] : Gg[pix]);
    }
  }

  int seedCount = FRAME_HEIGHT >> 2;
  int douseCount = seedCount >> 1;
  int xmax = FRAME_WIDTH - 1;

  for (int i=0; i<seedCount; i++)
    gGlowBuffer[ (FRAME_WIDTH * FRAME_HEIGHT) + (gRNG.genUInt32(0, xmax)) ] = (int8_t)gRNG.genInt32(10, 28);

  for (int i=0; i<douseCount; i++)
    gGlowBuffer[ (gRNG.genUInt32(0, xmax) * FRAME_HEIGHT) + (gRNG.genUInt32(0, xmax)) ] = 0;

  return true;
}
