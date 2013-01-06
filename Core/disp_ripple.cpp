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

struct RippleData
{
  enum
  {
    Width = Constants::FrameWidth + 2,
    Height = Constants::FrameHeight + 2,
    BufferSize = (Width * Height)
  };

  uint32_t  dropFrequency;
  Fix16     damping;
  Fix16     buffer[BufferSize * 2];
  Fix16*    source;
  Fix16*    destination;
};


// ---------------------------------------------------------------------------------------------------------------------
void ripple_init(FXState& state)
{
  RippleData* data = (RippleData*)state.store;

  Fix16 zero = fix16_from_int(0);

  data->dropFrequency = 3; // percent
  data->damping = 0.92f;
  for(int i = 0; i < RippleData::BufferSize * 2; ++i)
  {
    data->buffer[i] = zero;
  }
  data->source = data->buffer;
  data->destination = &data->buffer[RippleData::BufferSize];
}


// ---------------------------------------------------------------------------------------------------------------------
bool ripple_tick(FrameOutput& output, FXState& state)
{
  RippleData* data = (RippleData*)state.store;


  Fix16 zero = fix16_from_int(0);
  Fix16 divFour = fix16_from_float(0.1470588f);
  Fix16 edgeMult = fix16_from_float(0.8f);

  if(state.rng.genUInt32(0, 100) < data->dropFrequency)
  {
    data->source[state.rng.genUInt32(1, RippleData::Height-2) * RippleData::Width + state.rng.genUInt32(1, RippleData::Width-2)] = fix16_one;
  }

  for(int x = 1; x < RippleData::Width - 1; x++) 
  {
    for(int y = 1; y < RippleData::Height - 1; y++)
    {
      // last last height as proxy for velocity.
      Fix16 velocity = -data->destination[y * RippleData::Width + x];

      // average local neighborhood.
      // todo taking diagonal neigbors x 0.7 makes stuff rounder.
      Fix16 smoothed;
      smoothed = data->source[y * RippleData::Width + x + 1];   // x+1, y
      smoothed += data->source[y * RippleData::Width + x - 1];   // x-1, y
      smoothed += data->source[(y + 1) * RippleData::Width + x]; // x, y+1
      smoothed += data->source[(y - 1) * RippleData::Width + x]; // x, y-1

      smoothed += data->source[(y - 1) * RippleData::Width + (x - 1)] * edgeMult;
      smoothed += data->source[(y - 1) * RippleData::Width + (x + 1)] * edgeMult;
      smoothed += data->source[(y + 1) * RippleData::Width + (x - 1)] * edgeMult;
      smoothed += data->source[(y + 1) * RippleData::Width + (x + 1)] * edgeMult;

      smoothed *= divFour;

      // combine avg and velocity
      Fix16 newHeight = smoothed * fix16_from_float(2.0f) + velocity;
      
      // damp it
      newHeight *= data->damping;
      
      // store it...
      data->destination[y * RippleData::Width + x] = newHeight;
      
      // adjustments for drawing...
      newHeight = (newHeight * fix16_from_float(2.5f));

      // are there fix16 clamping functions someplace?
      if(newHeight < zero)
        newHeight = zero;
      if(newHeight > fix16_one)
        newHeight = fix16_one;

      byte c = (byte)fix16_to_int(newHeight * fix16_from_int(15));
      
      byte r, g;
      GetBasicColour(newHeight, Green, r, g);
      setLED(output.frame, x-1, y-1, r, g, 0);
    }
  }

  Fix16* temp = data->destination;
  data->destination = data->source;
  data->source = temp;

  return true;
}