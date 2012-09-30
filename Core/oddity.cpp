/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/

#include "oddity.h"
#include "mode.h"
#include "vfx.h"

#ifdef WIN32
#include <time.h>
#endif 

RNG gRNG(5);

static DisplayMode* gModeList[Mode::Count];
static Mode::Enum   gMode;

static uint32_t   gFrameCounter = 0;

// ---------------------------------------------------------------------------------------------------------------------
void oddity_init()
{
  gModeList[Mode::Boot] = new BootMode();
  gModeList[Mode::Flame] = new FlameMode();
  gModeList[Mode::Plasma] = new PlasmaMode();
  gModeList[Mode::Storm] = new StormMode();
  
  gMode = Mode::Storm;

#ifdef WIN32
  gRNG.reseed((uint32_t)time (NULL));
#endif

  gModeList[gMode]->init();
}

// ---------------------------------------------------------------------------------------------------------------------
bool oddity_tick(byte* frame, const hw_inputs& inputs)
{
  State tickState;
  tickState.m_frame = frame;
  tickState.m_inputs = inputs;
  tickState.m_counter = gFrameCounter ++;

  if (!gModeList[gMode]->tick(tickState))
  {
    gMode = (Mode::Enum)( (gMode + 1) % Mode::Count );
    gModeList[gMode]->init();

    gFrameCounter = 0;
  }

  return true;
}
