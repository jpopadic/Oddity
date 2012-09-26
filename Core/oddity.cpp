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


RNG gRNG(1024);

static DisplayMode* gModeList[Mode::Count];
static Mode::Enum   gMode;

static uint32_t   gFrameCounter = 0;

// ---------------------------------------------------------------------------------------------------------------------
void oddity_init()
{
  gModeList[Mode::Boot] = new BootMode();

  gMode = Mode::Boot;

  gModeList[gMode]->init();
}

// ---------------------------------------------------------------------------------------------------------------------
bool oddity_tick(byte* frame, const hw_inputs& inputs)
{
  State tickState;
  tickState.m_frame = frame;
  tickState.m_inputs = inputs;
  tickState.m_counter = gFrameCounter ++;

  gModeList[gMode]->tick(tickState);

  return true;
}
