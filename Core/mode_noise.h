/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/

#pragma once
#include "oddity.h"
#include "vfx.h"

// ---------------------------------------------------------------------------------------------------------------------
class NoiseMode : public DisplayMode
{
public:

  virtual void init();
  virtual bool tick(State &state);
};