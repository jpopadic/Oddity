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
#include "mode_boot.h"


TextScroller gBootScroller;

// ---------------------------------------------------------------------------------------------------------------------
void BootMode::init()
{
  gBootScroller.Init(" THIS IS AWESOME! ");
}

// ---------------------------------------------------------------------------------------------------------------------
bool BootMode::tick(State &state)
{
  return gBootScroller.Render(state.m_frame, Green);
}
