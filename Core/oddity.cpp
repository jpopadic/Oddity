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
  _act(ripple) \
  _act(plasma)


// ---------------------------------------------------------------------------------------------------------------------
// enum magic to turn the list above into useful code / ids / etc
#define PREDEC(_disp) \
  extern void _disp##_init(FXState& state); \
  extern bool _disp##_tick(FrameOutput& output, FXState& state);

#define ENUMID(_disp) \
  _disp,

#define DOINIT(_disp) \
  case _disp: _disp##_init(state); break;
#define DOTICK(_disp) \
  case _disp: _disp##_tick(output, state); break;

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
      default:
        break;
    }
  }
  static void doTickFor(Enum e, FrameOutput& output, FXState& state)
  {
    switch (e)
    {
      DISP(DOTICK)
      default:
        break;
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
  //DisplayMode::doTickFor(gCurrentDisplayMode, output, state);


  Fix16 pT = fix16_mul(fix16_from_int(state.counter), fix16_from_float(0.35f));
  Fix16 pY = fix16_mul(fix16_from_int(state.counter), fix16_from_float(0.1f));

  for (uint32_t x=0; x<Constants::FrameWidth; x++)
  {
    Fix16 pX = fix16_mul(fix16_from_int(x), fix16_from_float(0.3f));
    pX += pT;

    Fix16 row = Perlin2(pX, pY);
    row *= 17.0f;

    byte A[] = { 1, 2, 3, 3, 3, 3, 2, 1, 0 };
    byte B[] = { 0, 0, 0, 1, 2, 3, 3, 3, 3 };

    setLED(output.frame, x, 8, 1, 0);

    int rowInt = row.asInt();
    if (rowInt < 0)
      rowInt = -rowInt;
    if (rowInt > 8)
      rowInt = 8;

    for (uint32_t y=0; y<rowInt; y++)
    {
      setLED(output.frame, x, 8 - y, A[y], B[y]);
      setLED(output.frame, x, 8 + y, A[y], B[y]);
    }
  }

  return true;
}

} // namespace vfx

