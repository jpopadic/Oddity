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

extern void ripple_init(FXState& state);
extern bool ripple_tick(FrameOutput& output, FXState& state);


namespace vfx
{

// ---------------------------------------------------------------------------------------------------------------------
void init(FXState& state)
{
  state.counter = 0;
  
  for(int i = 0; i < Constants::MemoryPool; ++i)
    state.store[i] = 0xFF;

  ripple_init(state);
}

// ---------------------------------------------------------------------------------------------------------------------
bool tick(const FrameInput& input, FXState& state, FrameOutput& output)
{
  ripple_tick(output, state);

  return true;
}

} // namespace vfx

