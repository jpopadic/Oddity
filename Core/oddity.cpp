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


// ---------------------------------------------------------------------------------------------------------------------
// list of all known display modes (fx modules)
#define DISP(_act) \
  _act(ripple)


// ---------------------------------------------------------------------------------------------------------------------
// enum magic to turn the list above into useful code / ids / etc
#define PREDEC(_disp) \
  extern void _disp##_init(FXState& state); \
  extern bool _disp##_tick(FrameOutput& output, FXState& state);

#define ENUMID(_disp) \
  _disp,

#define DOINIT(_disp) \
  case _disp: _disp##_init(state);
#define DOTICK(_disp) \
  case _disp: _disp##_tick(output, state);

DISP(PREDEC)
struct DisplayMode
{
  enum Enum
  {
    DISP(ENUMID)
    Count
  };

  static void doInitFor(Enum e, FXState& state)
  {
    switch (e)
    {
      DISP(DOINIT)      
    }
  }
  static void doTickFor(Enum e, FrameOutput& output, FXState& state)
  {
    switch (e)
    {
      DISP(DOTICK)      
    }
  }
};


// ---------------------------------------------------------------------------------------------------------------------
namespace vfx
{

DisplayMode::Enum gCurrentDisplayMode = DisplayMode::ripple;

// ---------------------------------------------------------------------------------------------------------------------
void init(FXState& state)
{
  state.counter = 0;
  
  for(int i = 0; i < Constants::MemoryPool; ++i)
    state.store[i] = 0xFF;

  DisplayMode::doInitFor(gCurrentDisplayMode, state);
}

// ---------------------------------------------------------------------------------------------------------------------
bool tick(const FrameInput& input, FXState& state, FrameOutput& output)
{
  DisplayMode::doTickFor(gCurrentDisplayMode, output, state);

  return true;
}

} // namespace vfx

