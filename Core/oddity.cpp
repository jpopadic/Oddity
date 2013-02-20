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

DisplayMode::Enum gCurrentDisplayMode = DisplayMode::plasma;

// ---------------------------------------------------------------------------------------------------------------------
void init(FXState& state)
{
  state.counter = 0;
  
  for(int i = 0; i < Constants::MemoryPool; ++i)
    state.store[i] = 0xFF;

  DisplayMode::doInitFor(gCurrentDisplayMode, state);
}

static int32_t dialA = 0;
static int32_t dialB = 0;

static Fix16 vTargetA = fix16_from_int(0), vCurA = fix16_from_int(0);
static Fix16 vTargetB = fix16_from_int(0), vCurB = fix16_from_int(0);

// ---------------------------------------------------------------------------------------------------------------------
bool tick(const FrameInput& input, FXState& state, FrameOutput& output)
{
//  DisplayMode::doTickFor(gCurrentDisplayMode, output, state);
/*
   dial --;
   if (dial <= 0)
   {
     // clear the frame
  output.clear();
   for (int y=0; y<Constants::FrameHeight; y++)
   {
     for (int x=0; x<Constants::FrameWidth; x++)
     {
       int32_t RR = state.rng.genInt32(-4, 4);
       if (RR<0)
       RR =0;
       int32_t GG = state.rng.genInt32(-4, 4);
       if (GG<0)
       GG =0;
     
       setLED(output.frame, x, y, RR, GG);
     }
   }
dial = 6;   
}  */

  byte red, green;
  for (int y=0; y<Constants::FrameHeight; y++)
  {
    for (int x=0; x<Constants::FrameWidth; x++)
    {
      pixel &LEDpixel = output.frame[y * Constants::FrameWidth + x];  
      
      DecodeByte(LEDpixel, red, green);
      if (red > 0)
        red --;
      if (green > 0)
        green --;

      LEDpixel = red | (green << 4);
    }
  }

  vTargetA += fix16_from_int( input.dialChange[1] );
  vTargetB += fix16_from_int( input.dialChange[2] );
  

  vCurA += (vTargetA - vCurA) * fix16_from_float(0.05f);
  vCurB += (vTargetB - vCurB) * fix16_from_float(0.05f);


  Fix16 xo = fix16_from_float(8.0f);
  Fix16 yo = fix16_from_float(8.0f);

  Fix16 ss1 = fix16_sin(vCurA * fix16_from_float(0.1f));
  Fix16 cc1 = fix16_cos(vCurA * fix16_from_float(0.1f));

  Fix16 ss2 = fix16_sin(vCurB * fix16_from_float(0.1f));
  Fix16 cc2 = fix16_cos(vCurB * fix16_from_float(0.1f));

  Fix16 rad1(-2.0f), rad2(12.0f);

  draw::WuLine(
    output.frame, 
    xo + (ss1 * rad1), 
    yo + (cc1 * rad1), 
    xo + (ss1 * rad2), 
    yo + (cc1 * rad2), 
    Red);

  draw::WuLine(
    output.frame, 
    xo + (ss2 * rad1), 
    yo + (cc2 * rad1), 
    xo + (ss2 * rad2), 
    yo + (cc2 * rad2), 
    Green);



//    Fix16 pT = fix16_mul(fix16_from_int(state.counter), fix16_from_float(0.1f));
//    Fix16 pY = fix16_mul(fix16_from_int(state.counter), fix16_from_float(0.03f));
//  
//    for (uint32_t x=0; x<Constants::FrameWidth; x++)
//    {
//      Fix16 pX = fix16_mul(fix16_from_int(x), fix16_from_float(0.3f));
//      pX += pT;
//  
//      Fix16 row = Perlin2(pX, pY);
//      row *= 17.0f;
//  
//      byte A[] = { 1, 2, 3, 3, 3, 2, 1, 0, 0 };
//      byte B[] = { 0, 0, 0, 1, 2, 3, 3, 3, 2 };
//  
//      setLED(output.frame, x, 8, 1, 0);
//  
//      int rowInt = row.asInt();
//      if (rowInt < 0)
//        rowInt = -rowInt;
//      if (rowInt > 8)
//        rowInt = 8;
//  
//      for (uint32_t y=0; y<rowInt; y++)
//      {
//        setLED(output.frame, x, 8 - y, A[y], B[y]);
//        setLED(output.frame, x, 8 + y, A[y], B[y]);
//      }
//    }

  return true;
}

} // namespace vfx

