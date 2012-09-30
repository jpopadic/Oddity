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
#include "mode_storm.h"

struct P2
{
  Fix16 x, y;
};

enum { NumParticles = 12 };
static P2 gParticlePos[NumParticles];
static P2 gParticleVel[NumParticles];
static bool gAlive[NumParticles];
static int gTTL[NumParticles];

// ---------------------------------------------------------------------------------------------------------------------
void StormMode::init()
{
  for (int i=0; i<NumParticles; i++)
    gAlive[i] = false;
}

// ---------------------------------------------------------------------------------------------------------------------
bool StormMode::tick(State &state)
{
  if (state.m_counter > 300)
    return false;

  Fix16 boundMin(0.0f), boundMax(16.0f), invert(-0.9f), decay(0.98f), vel(-2.5f);
  byte r, g;

  int spawnPerFrame = 2;

  for (int i=0; i<NumParticles; i++)
  {
    if (!gAlive[i])
    {
      spawnPerFrame --;
      if (spawnPerFrame > 0)
      {
        gParticlePos[i].x = 
          gParticlePos[i].y = 2.0f;

        gParticlePos[i].x += (fix16_t)gRNG.genUInt32(0, 0x5ffff); 
        gParticlePos[i].y += (fix16_t)gRNG.genUInt32(0, 0x5ffff);


        gParticleVel[i].x = (fix16_t)gRNG.genUInt32(0, 0x1ffff);
        gParticleVel[i].y = (fix16_t)gRNG.genUInt32(0, 0x1ffff);

        if (gRNG.genBool())
          gParticleVel[i].x *= invert;
        if (gRNG.genBool())
          gParticleVel[i].y *= invert;

        gAlive[i] = true;
        gTTL[i] = gRNG.genUInt32(40, 80);
      }
    }
    else
    {
      gParticlePos[i].x += gParticleVel[i].x;
      gParticlePos[i].y += gParticleVel[i].y;

      if (gParticlePos[i].x > boundMax)
      {
        gParticlePos[i].x = boundMax;
        gParticleVel[i].x *= invert;
      }
      if (gParticlePos[i].y > boundMax)
      {
        gParticlePos[i].y = boundMax;
        gParticleVel[i].y *= invert;
      }
      if (gParticlePos[i].x < boundMin)
      {
        gParticlePos[i].x = boundMin;
        gParticleVel[i].x *= invert;
      }
      if (gParticlePos[i].y < boundMin)
      {
        gParticlePos[i].y = boundMin;
        gParticleVel[i].y *= invert;
      }

      gParticleVel[i].x *= decay;
      gParticleVel[i].y *= decay;

      gParticleVel[i].y += 0.092f;

//       int _x = gParticlePos[i].x.asInt();
//       int _y = gParticlePos[i].y.asInt();
// 
//       Fix16 t = (gParticleVel[i].x + gParticleVel[i].y) * 0.5f;
//       ColourGradient(t, true, true, r, g);
// 
//       setLED(state.m_frame, _x, _y, 3, 1, true);

      Fix16 _x = gParticlePos[i].x + (gParticleVel[i].x * vel);
      Fix16 _y = gParticlePos[i].y + (gParticleVel[i].y * vel);

      draw::WuLine(state.m_frame, gParticlePos[i].x, gParticlePos[i].y, _x, _y, Orange);

      gTTL[i] --;
      if (gTTL[i] <= 0)
        gAlive[i] = false;
    }
  }

  return true;
}
