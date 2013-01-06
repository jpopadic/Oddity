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

#ifdef WIN32
#include <time.h>
#endif 

namespace vfx
{

// ---------------------------------------------------------------------------------------------------------------------
void init(FXState& state)
{
  state.counter = 0;
  
  for(int i = 0; i < Constants::MemoryPool; ++i)
    state.store[i] = 0xFF;
}

// ---------------------------------------------------------------------------------------------------------------------
bool tick(const FrameInput& input, FXState& state, FrameOutput& output)
{
  static uint16_t ff = 0;
  ff += input.dialChange[0];

  draw::FontGlyph8x8(output.frame, 'A', ff, 0, Lime);
  draw::FontGlyph8x8(output.frame, '6', 8, 0, Lime);
  draw::FontGlyph8x8(output.frame, '0', 0, 8, Lime);
  draw::FontGlyph8x8(output.frame, '1', 8, 8, Lime);
  return true;
}

} // namespace vfx

