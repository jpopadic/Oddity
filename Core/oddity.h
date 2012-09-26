/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/

#pragma once
#include <stdlib.h>
#include "fixedpoint.h"
#include "RNG.h"

//
// assumes fixed 16x16 RG configuration, 2-bits per pixel.
//
// frame stored 1-byte per pixel (XXRR XXGG)
#define FRAME_WIDTH  16
#define FRAME_HEIGHT 16
#define FRAME_SIZE FRAME_WIDTH * FRAME_HEIGHT

#ifdef _MSC_VER
#include <stddef.h>
#endif 
typedef unsigned char byte;

extern RNG gRNG;

struct hw_inputs
{
  int encoderChange[3];
};

void oddity_init();
bool oddity_tick(byte* frame, const hw_inputs& inputs);


// ---------------------------------------------------------------------------------------------------------------------
struct State
{
  byte         *m_frame;

  hw_inputs     m_inputs;
  uint32_t      m_counter;
};


// ---------------------------------------------------------------------------------------------------------------------
namespace Mode
{
  enum Enum
  {
    Boot,

    Count
  };
}

// ---------------------------------------------------------------------------------------------------------------------
class DisplayMode
{
public:

  virtual void init()=0;
  virtual bool tick(State &state)=0;
};


