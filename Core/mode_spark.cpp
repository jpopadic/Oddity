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
#include "mode_spark.h"

struct P2
{
  Fix16 x, y;
};

enum { NumParticles = 12 };
static P2 gParticlePos[NumParticles];
static P2 gParticleVel[NumParticles];
static bool gAlive[NumParticles];
static int gTTL[NumParticles];
static bool gGravity;

// ---------------------------------------------------------------------------------------------------------------------
void SparkMode::init()
{
  for (int i=0; i<NumParticles; i++)
    gAlive[i] = false;

  gGravity = true;//gRNG.genBool();
}

// ---------------------------------------------------------------------------------------------------------------------
bool SparkMode::tick(State &state)
{
  if (state.m_counter > 300)
    return false;

  Fix16 boundMin(0.0f), boundMax(16.0f), invert(-0.9f), decay(0.98f), invertDecayed(-0.8f), vel(2.0f);

  int spawnPerFrame = 2;

  for (int i=0; i<NumParticles; i++)
  {
    if (!gAlive[i])
    {
      spawnPerFrame --;
      if (spawnPerFrame > 0)
      {
        gParticlePos[i].x = fix16_t(0);
        gParticlePos[i].y = 8.0f;


        gParticleVel[i].x = (fix16_t)gRNG.genUInt32(0, 0x1ffff);
        gParticleVel[i].y = (fix16_t)gRNG.genUInt32(0, 0x1ffff);

         gParticleVel[i].y *= invert;

        gAlive[i] = true;
        gTTL[i] = gRNG.genUInt32(20, 40);
      }
    }
    else
    {
      gParticlePos[i].x += gParticleVel[i].x;
      gParticlePos[i].y += gParticleVel[i].y;

      if (gGravity)
      {
        if (gParticlePos[i].x > boundMax)
        {
          gParticlePos[i].x = boundMax;
          gParticleVel[i].x *= invert;
        }
        if (gParticlePos[i].y > boundMax)
        {
          gParticlePos[i].y = boundMax;
          gParticleVel[i].y *= invertDecayed;
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
      }

      gParticleVel[i].x *= decay;
      gParticleVel[i].y *= decay;

      if (gGravity)
        gParticleVel[i].y += 0.08f;


      Fix16 _x = gParticlePos[i].x + (gParticleVel[i].x * vel);
      Fix16 _y = gParticlePos[i].y + (gParticleVel[i].y * vel);

      draw::WuLine(state.m_frame, gParticlePos[i].x, gParticlePos[i].y, _x, _y, ((i & 1) ? Lime : Green) );

      gTTL[i] --;
      if (gTTL[i] <= 0)
        gAlive[i] = false;
    }
  }

  return true;
}
