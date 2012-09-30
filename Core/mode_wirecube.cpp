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
#include "mode_wirecube.h"

static const Fxp3D verts[8] = {
  Fxp3D(-1.0f,  1.0f, -1.0f),
  Fxp3D( 1.0f,  1.0f, -1.0f),
  Fxp3D( 1.0f, -1.0f, -1.0f),
  Fxp3D(-1.0f, -1.0f, -1.0f),

  Fxp3D(-1.0f,  1.0f,  1.0f),
  Fxp3D( 1.0f,  1.0f,  1.0f),
  Fxp3D( 1.0f, -1.0f,  1.0f),
  Fxp3D(-1.0f, -1.0f,  1.0f),
};

static const int faces[6 * 4] = {
  0,1,2,3,
  1,5,6,2,
  5,4,7,6,
  4,0,3,7,
  0,4,5,1,
  3,2,6,7
};

static Fix16 rotX, rotY, rotZ, distPulse;
static ColourChoice ccCycle;

// ---------------------------------------------------------------------------------------------------------------------
void WirecubeMode::init()
{
  rotX = (fix16_t)gRNG.genUInt32(0, 0xfffff);
  rotY = (fix16_t)gRNG.genUInt32(0, 0xfffff);
  rotZ = (fix16_t)gRNG.genUInt32(0, 0xfffff);
  distPulse = 0.0f;

  ccCycle = (ColourChoice)gRNG.genUInt32(0, Yellow);
}

// ---------------------------------------------------------------------------------------------------------------------
bool WirecubeMode::tick(State &state)
{
  if (state.m_counter > 300)
    return false;

  Fxp3D pj[8];

  Fix16 fov(9.0f), dist;

  dist = dist.cos(distPulse);
  dist *= 0.5f;
  dist += 2.2f;


  for (int i=0; i<8; i++)
  {
    pj[i] = verts[i].eulerProject(rotX, rotY, rotZ, fov, dist);
  }

  rotX += 1.0f;
  rotY += 2.0f;
  rotZ += 0.5f;
  distPulse += 0.07f;

  for (int f=0; f<6; f++)
  {
    int bi = (f * 4);
    draw::WuLine(state.m_frame,
      pj[ faces[bi + 0] ].x,
      pj[ faces[bi + 0] ].y,
      pj[ faces[bi + 1] ].x,
      pj[ faces[bi + 1] ].y,
      ccCycle);

    draw::WuLine(state.m_frame,
      pj[ faces[bi + 1] ].x,
      pj[ faces[bi + 1] ].y,
      pj[ faces[bi + 2] ].x,
      pj[ faces[bi + 2] ].y,
      ccCycle);

    draw::WuLine(state.m_frame,
      pj[ faces[bi + 2] ].x,
      pj[ faces[bi + 2] ].y,
      pj[ faces[bi + 3] ].x,
      pj[ faces[bi + 3] ].y,
      ccCycle);

    draw::WuLine(state.m_frame,
      pj[ faces[bi + 3] ].x,
      pj[ faces[bi + 3] ].y,
      pj[ faces[bi + 0] ].x,
      pj[ faces[bi + 0] ].y,
      ccCycle);
  }

  return true;
}
