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


uint32_t MurmurHash2(const void *key, uint32_t len, uint32_t seed)
{
  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.
  const uint32_t m = 0x5bd1e995;
  const int32_t r = 24;

  // Initialize the hash to a 'random' value
  uint32_t h = seed ^ len;

  // Mix 4 bytes at a time into the hash
  const uint8_t *data = (const uint8_t *)key;

  while(len >= 4)
  {
    uint32_t k = *(const uint32_t *)data;       //lint !e826 cast to ptr of larger size

    k *= m; 
    k ^= k >> r; 
    k *= m; 

    h *= m; 
    h ^= k;

    data += 4;
    len -= 4;
  }

  // Handle the last few bytes of the input array

  switch(len)
  {
    //lint --e{616, 825} // control flows into case/default
  case 3: h ^= data[2] << 16;
  case 2: h ^= data[1] << 8;
  case 1: h ^= data[0];
    h *= m;

  }; //lint !e744 no default

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
} 

// ---------------------------------------------------------------------------------------------------------------------
// list of all known display modes (fx modules)
#define DISP(_act) \
  _act(ripple) \
  _act(plasma) \
  _act(cube) \
  _act(eq) \
  _act(rotozoom) \
  _act(fire)


// ---------------------------------------------------------------------------------------------------------------------
// enum magic to turn the list above into useful code / ids / etc
#define PREDEC(_disp) \
  extern void _disp##_init(FXState& state); \
  extern bool _disp##_tick(const FrameInput& input, FrameOutput& output, FXState& state);

#define ENUMID(_disp) \
  _disp,

#define DOINIT(_disp) \
  case _disp: _disp##_init(state); break;
#define DOTICK(_disp) \
  case _disp: _disp##_tick(input, output, state); break;

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
  static void doTickFor(Enum e, const FrameInput& input, FrameOutput& output, FXState& state)
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

struct InterfaceStage 
{
  enum Enum
  {
    IS_GUI,
    IS_FADETO,
    IS_FX,
    IS_FADEFROM
  };
};

DisplayMode::Enum gCurrentDisplayMode = DisplayMode::plasma;
InterfaceStage::Enum gCurrentInterfaceStage = InterfaceStage::IS_GUI;
DisplayMode::Enum gNextDisplayMode = DisplayMode::plasma;
int16_t fadeTick = 0;

// ---------------------------------------------------------------------------------------------------------------------
void init(FXState& state)
{
  state.counter = 0;

  state.rng.reseed(MurmurHash2(__TIMESTAMP__, sizeof(__TIMESTAMP__), 0xb33f));
}

static int32_t dialA = 0;
static int32_t dialB = 0;

static Fix16 vTargetA = fix16_from_int(0), vCurA = fix16_from_int(0);
static Fix16 vTargetB = fix16_from_int(0), vCurB = fix16_from_int(0);

struct gui_entry
{
  int16_t offset;
  ColourChoice cc;
  DisplayMode::Enum mode;
};

static const char radioText[] = "00.01.02.03";
static const gui_entry radioGUI[] = 
{
  {3, Lime, DisplayMode::plasma},
  {6, Orange, DisplayMode::cube},
  {9, Red, DisplayMode::fire},
  {0xff, Green, DisplayMode::eq},
};
static const int16_t radioGUICount = sizeof(radioGUI) / sizeof(gui_entry);

// ---------------------------------------------------------------------------------------------------------------------
bool tick(const FrameInput& input, FXState& state, FrameOutput& output)
{
  switch (gCurrentInterfaceStage)
  {
  case InterfaceStage::IS_GUI:
    {
      output.clear();

      const int16_t radioLen = sizeof(radioText) - 1;

      Fix16 max_target = fix16_from_int(radioLen);
      Fix16 max_value = fix16_from_int(radioLen - 1);

      Fix16 dial16 = fix16_from_int( input.dialChange[1] );
      Fix16 pt05 = fix16_from_float(-1.0f);

      vTargetA += dial16 * fix16_from_float(0.2f);
      if (vTargetA < fix16_neg_one)
        vTargetA = fix16_neg_one;

      if (vTargetA > max_target)
        vTargetA = max_target;

      vCurA += (vTargetA - vCurA) * fix16_from_float(0.01f);

      if (vCurA < fix16_zero)
        vCurA = fix16_zero;
      if (vCurA > max_value)
        vCurA = max_value;

      int16_t off = ipart(vCurA).asInt();
      if (off < 0)
        off = 0;
      if (off > radioLen - 1)
        off = radioLen - 1;

      const gui_entry* guient[3] = {0, 0, 0};
      for (int16_t q=0; q<radioGUICount; q++)
      {
        if (off < radioGUI[q].offset)
        {
          guient[0] = &radioGUI[q];

          for (int16_t q=0; q<radioGUICount; q++)
          {
            if (off - 1 < radioGUI[q].offset)
            {
              guient[2] = &radioGUI[q];
              break;
            }
          }
          for (int16_t q=0; q<radioGUICount; q++)
          {
            if (off + 1 < radioGUI[q].offset)
            {
              guient[1] = &radioGUI[q];
              break;
            }
          }

          break;
        }
      }


      Fix16 charASlide = fpart(vCurA);


      charASlide *= Fix16(-16.0f);
      int16_t slideAInt = charASlide.asInt();

      draw::FontGlyph16x16(output.frame, radioText[off], slideAInt, 0, guient[0]->cc);

      draw::FontGlyph16x16(output.frame, radioText[off + 1], 16 + slideAInt, 0, guient[1]->cc);

      if (off > 0)
        draw::FontGlyph16x16(output.frame, radioText[off - 1], slideAInt - 16, 0, guient[2]->cc);

      if (fadeTick < 15)
      {
        fadeTick ++;
        output.fade(15 - fadeTick);
      }

      if (input.dialClick)
      {
        gCurrentInterfaceStage = InterfaceStage::IS_FADETO;

        gNextDisplayMode = guient[0]->mode;
      }
    }
    break;

  case InterfaceStage::IS_FADETO:
    {
      output.fade(1);

      fadeTick --;
      if (fadeTick <= 0)
      {
        vTargetA = vCurA;

        gCurrentDisplayMode = gNextDisplayMode;

        for(int i = 0; i < Constants::MemoryPool; ++i)
          state.store[i] = 0xFF;

        DisplayMode::doInitFor(gCurrentDisplayMode, state);
        fadeTick = 0;

        gCurrentInterfaceStage = InterfaceStage::IS_FX;
      }
    }
    break;

  case InterfaceStage::IS_FX:
    {
      DisplayMode::doTickFor(gCurrentDisplayMode, input, output, state);

      if (fadeTick < 15)
      {
        fadeTick ++;
        output.fade(15 - fadeTick);
      }

      if (input.dialClick)
      {
        gCurrentInterfaceStage = InterfaceStage::IS_FADEFROM;
      }
    }
    break;

  case InterfaceStage::IS_FADEFROM:
    {
      DisplayMode::doTickFor(gCurrentDisplayMode, input, output, state);

      output.fade(15 - fadeTick);

      fadeTick --;
      if (fadeTick <= 0)
      {
        gCurrentInterfaceStage = InterfaceStage::IS_GUI;
        fadeTick = 0;
      }
    }
    break;

  }

//  
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



  /*
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

  */



  return true;
}

} // namespace vfx

