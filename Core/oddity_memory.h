/*
                 __    ___ __       
      ____  ____/ /___/ (_) /___  __
     / __ \/ __  / __  / / __/ / / /    [ LED matrix VFX core ] 
    / /_/ / /_/ / /_/ / / /_/ /_/ / 
    \____/\__,_/\__,_/_/\__/\__, /     [ john & harry ] [ 2012 ]
                           /____/   
*/

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
#include "fixedpoint.h"
#include "RNG.h"

#ifdef _MSC_VER
#include <stddef.h>
#endif 
typedef unsigned char byte;


// ---------------------------------------------------------------------------------------------------------------------
struct Constants
{
  enum Enum
  {
    // output configuration
    FrameWidth  = 16,
    FrameHeight = 16,
    FrameSize   = FrameWidth * FrameHeight,

    // size of the shared global memory pool
    MemoryPool  = 32 * 1024
  };
};


// ---------------------------------------------------------------------------------------------------------------------
//
struct FrameInput
{
  // 1 or -1 denotes a change in dial position
  int   dialChange[3];
};

struct FrameOutput
{
  // assumes fixed 16x16 RG configuration, 2-bits per pixel.
  // frame stored 1-byte per pixel (XXRR XXGG)
  byte  frame[Constants::FrameSize];

  void clear()
  {
    for(uint32_t i = 0; i < Constants::FrameSize; ++i)
      frame[i] = 0;
  }
};

// ---------------------------------------------------------------------------------------------------------------------
// 
struct FXState
{
  uint32_t      counter;
  RNG           rng;

  byte          store[Constants::MemoryPool];
};