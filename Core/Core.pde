#include <stdlib.h>
#include <unistd.h>
#include "oddity.h"

//----------------------------------------------------------------------------------------------------------------------
// hax
extern "C"
{
int _getpid( void ) { return 0; }
void _exit(int status) {}
int _kill(pid_t pid, int sig) { return 0; }
}

//----------------------------------------------------------------------------------------------------------------------
HardwareSPI spi(1);

HardwareTimer displayTimer(1);
HardwareTimer frameTimer(2);


enum Pins
{
  // column data pins
  LE = 9,
  OE = 8,

  // column driver pins
  CLK = 7,
  DATA = 6,
  STROBE = 5
};

#define FRAME_TIME 60


//----------------------------------------------------------------------------------------------------------------------
//
// frame buffer stored as 6 bitmaps (2 colors x 3 color
// intensities) ordered: R0 R1 R2 G0 G1 G2
//
// 32 bytes per pixel per color
#define BITMAP_SIZE  (Constants::FrameWidth * Constants::FrameHeight / 8)
#define BUFFER_SIZE  (BITMAP_SIZE * 6)
#define GREEN_OFFSET (BITMAP_SIZE * 3)

// variables for display update
int gIntensity;
int gColumn;
int gIntensityMap[] = { 50, 100, 150 }; // geometric 50, 150, 300


//----------------------------------------------------------------------------------------------------------------------
// double buffer
byte buffer[BUFFER_SIZE * 2];
byte* pFrontBuffer;
byte* pBackBuffer;

inline void clearBuffer(byte* buffer)
{
  for(int i = 0; i < BUFFER_SIZE; ++i)
    buffer[i] = 0;
}

inline void flipBuffer()
{
  byte* temp = pFrontBuffer;
  pFrontBuffer = pBackBuffer;
  pBackBuffer = temp; 
}


//----------------------------------------------------------------------------------------------------------------------
// frame
void loadFrame(byte* buffer, byte* frame);
volatile boolean gHaveNewFrame;

//----------------------------------------------------------------------------------------------------------------------
// encoder inputs
#define ENCODER_A_PIN_1 15
#define ENCODER_A_PIN_2 16
#define ENCODER_B_PIN_1 18
#define ENCODER_B_PIN_2 20
#define ENCODER_C_PIN_1 17
#define ENCODER_C_PIN_2 19

void handleEncoderA();
void handleEncoderB();
void handleEncoderC();
volatile int analogIncA, analogIncB, analogIncC;
int analogLastA, analogLastB, analogLastC;

//----------------------------------------------------------------------------------------------------------------------
static FrameOutput g_output;
static FXState     g_state;


//----------------------------------------------------------------------------------------------------------------------
// main code
void setup()
{
  initialize();
  g_output.clear();
  
  vfx::init(g_state);
}

//----------------------------------------------------------------------------------------------------------------------
void loop()
{
  if (gHaveNewFrame)
    return;
  
  FrameInput inputs;

  // work out changes from the encoders
  int alA = analogIncA;
  int alB = analogIncB;
  int alC = analogIncC;  
  inputs.dialChange[0] = (alA - analogLastA);
  inputs.dialChange[1] = (alB - analogLastB);
  inputs.dialChange[2] = (alC - analogLastC);
  analogLastA = alA;
  analogLastB = alB;
  analogLastC = alC;
  

  g_state.counter ++;

  // clear the frame
  g_output.clear();
  
  // tick the VFX core
  if (vfx::tick(inputs, g_state, g_output))
  {
    clearBuffer(pBackBuffer);
    loadFrame(pBackBuffer, g_output.frame);
    gHaveNewFrame = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void updateFrame()
{
  if(gHaveNewFrame == true)
  {
    gHaveNewFrame = false;
    flipBuffer();
  }
  
  frameTimer.pause(); 
  frameTimer.setPeriod(FRAME_TIME * 1000); // 20ms in us
  frameTimer.refresh();
  frameTimer.resume();  
}

//----------------------------------------------------------------------------------------------------------------------
void initialize()
{
  // set up some outputs
  pinMode(LE, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(STROBE, OUTPUT);
  
  // initialize outputs
  digitalWrite(LE, 0);    // init row latch low
  digitalWrite(OE, 1);    // disable row drivers
  digitalWrite(CLK, 0);   // init column clock low
  digitalWrite(DATA, 0);  // init column data low
  digitalWrite(STROBE, 0);// init column strobe low
  
  // clear the column driver
  digitalWrite(DATA, 1);
  for(int i = 0; i < 16; i++)
  {
    digitalWrite(CLK, 1);
    digitalWrite(CLK, 0);
  }

  // Turn on the SPI port
  spi.begin(SPI_18MHZ, MSBFIRST, 0);
  
  // todo figure out how to know when write complete.
  // 4 nops appear to be about right.
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  
  // latch row data
  digitalWrite(LE, 1);
  digitalWrite(LE, 0);
  
  // set up frame buffer
  pFrontBuffer = buffer;
  pBackBuffer = buffer + BUFFER_SIZE;
  clearBuffer(pFrontBuffer);
  clearBuffer(pBackBuffer);
  
  analogIncA = analogLastA =0;
  analogIncB = analogLastB =0;
  analogIncC = analogLastC =0;
  
  
  gHaveNewFrame = false;

  // encoder interrupts
  pinMode(ENCODER_A_PIN_1, INPUT_PULLUP);
  pinMode(ENCODER_A_PIN_2, INPUT_PULLUP);
  attachInterrupt(ENCODER_A_PIN_1, handleEncoderA, CHANGE);
  pinMode(ENCODER_B_PIN_1, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN_2, INPUT_PULLUP);
  attachInterrupt(ENCODER_B_PIN_1, handleEncoderB, CHANGE);
  pinMode(ENCODER_C_PIN_1, INPUT_PULLUP);
  pinMode(ENCODER_C_PIN_2, INPUT_PULLUP);
  attachInterrupt(ENCODER_C_PIN_1, handleEncoderC, CHANGE);
    
  //
  // set up frame timer
  frameTimer.pause();
  frameTimer.setPeriod(FRAME_TIME * 1000);
  frameTimer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  frameTimer.setCompare(TIMER_CH2, 1);
  frameTimer.attachCompare1Interrupt(updateFrame);
  frameTimer.refresh();
  frameTimer.resume();
  
  //
  // set up display timer
  gIntensity = 0;
  gColumn = 0;

  displayTimer.pause(); 
  displayTimer.setPeriod(gIntensityMap[gIntensity]);
  displayTimer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  displayTimer.setCompare(TIMER_CH1, 1);
  displayTimer.attachCompare1Interrupt(updateDisplay);
  displayTimer.refresh();
  displayTimer.resume();  
}

//----------------------------------------------------------------------------------------------------------------------
void updateDisplay()
{
  // disable row drivers
  digitalWrite(OE, 1);

  // increment column driver
  // if this is the first column, seed with (inverted) on-state.
  // this "on" will be shifted left for the next 15 columns.
  if(gColumn != 0)
    digitalWrite(DATA, 1);
  else
    digitalWrite(DATA, 0);
  
  // clock data in
  digitalWrite(CLK, 1);
  digitalWrite(CLK, 0);

  // latch shifted data
  digitalWrite(STROBE, 1);    
  digitalWrite(STROBE, 0);
  
  // push row data
  const int intensityOffset = gIntensity * BITMAP_SIZE;
  const int columnOffset    = gColumn * 2;

  // red first
  spi.write(pFrontBuffer[intensityOffset + columnOffset + 1]);
  spi.write(pFrontBuffer[intensityOffset + columnOffset]);

  // then green
  spi.write(pFrontBuffer[GREEN_OFFSET + intensityOffset + columnOffset + 1]);
  spi.write(pFrontBuffer[GREEN_OFFSET + intensityOffset + columnOffset]);
    

  // todo figure out how to know when write complete.
  // 4 nops appear to be about right.
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop"); 
 
  // latch row data
  digitalWrite(LE, 1);
  digitalWrite(LE, 0);
  
  // update up display timer with delay for this intensity
  displayTimer.pause(); 
  displayTimer.setPeriod(gIntensityMap[gIntensity]);
  displayTimer.refresh();
  displayTimer.resume();
  
  // enable row drivers
  digitalWrite(OE, 0);
  
  if(++gColumn > 15)
  {
    gColumn = 0;
    if(++gIntensity > 2)
    {
      gIntensity = 0;  
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
void handleEncoderA()
{
  // pin ENCODER_A_PIN has changed, so something has happened.
  // ENCODER_B_PIN determines direction.
  if(digitalRead(ENCODER_A_PIN_1) == digitalRead(ENCODER_A_PIN_2))
  {
    analogIncA ++;
  }
  else
  {
    analogIncA --;
  }
}
void handleEncoderB()
{
  if(digitalRead(ENCODER_B_PIN_1) == digitalRead(ENCODER_B_PIN_2))
  {
    analogIncB ++;
  }
  else
  {
    analogIncB --;
  }
}
void handleEncoderC()
{
  if(digitalRead(ENCODER_C_PIN_1) == digitalRead(ENCODER_C_PIN_2))
  {
    analogIncC ++;
  }
  else
  {
    analogIncC --;
  }
}

//----------------------------------------------------------------------------------------------------------------------
const static byte addressing_red[Constants::FrameSize * 5] = { 
0, 1, 16, 48, 80, 129, 2, 0, 32, 64, 2, 4, 16, 48, 80, 131, 8, 0, 32, 64, 4, 16, 16, 48, 80, 133, 32, 0, 32, 64, 6, 64, 16, 48, 80, 135, 128, 0, 32, 64, 8, 1, 17, 49, 81, 137, 2, 1, 33, 65, 10, 4, 17, 49, 81, 139, 8, 1, 33, 65, 12, 16, 17, 49, 81, 141, 32, 1, 33, 65, 14, 64, 17, 49, 81, 
143, 128, 1, 33, 65, 16, 1, 18, 50, 82, 145, 2, 2, 34, 66, 18, 4, 18, 50, 82, 147, 8, 2, 34, 66, 20, 16, 18, 50, 82, 149, 32, 2, 34, 66, 22, 64, 18, 50, 82, 151, 128, 2, 34, 66, 24, 1, 19, 51, 83, 153, 2, 3, 35, 67, 26, 4, 19, 51, 83, 155, 8, 3, 35, 67, 28, 16, 19, 51, 83, 157, 32, 3, 35, 67, 30, 64, 19, 51, 83, 
159, 128, 3, 35, 67, 32, 1, 20, 52, 84, 161, 2, 4, 36, 68, 34, 4, 20, 52, 84, 163, 8, 4, 36, 68, 36, 16, 20, 52, 84, 165, 32, 4, 36, 68, 38, 64, 20, 52, 84, 167, 128, 4, 36, 68, 40, 1, 21, 53, 85, 169, 2, 5, 37, 69, 42, 4, 21, 53, 85, 171, 8, 5, 37, 69, 44, 16, 21, 53, 85, 173, 32, 5, 37, 69, 46, 64, 21, 53, 85, 
175, 128, 5, 37, 69, 48, 1, 22, 54, 86, 177, 2, 6, 38, 70, 50, 4, 22, 54, 86, 179, 8, 6, 38, 70, 52, 16, 22, 54, 86, 181, 32, 6, 38, 70, 54, 64, 22, 54, 86, 183, 128, 6, 38, 70, 56, 1, 23, 55, 87, 185, 2, 7, 39, 71, 58, 4, 23, 55, 87, 187, 8, 7, 39, 71, 60, 16, 23, 55, 87, 189, 32, 7, 39, 71, 62, 64, 23, 55, 87, 
191, 128, 7, 39, 71, 64, 1, 24, 56, 88, 193, 2, 8, 40, 72, 66, 4, 24, 56, 88, 195, 8, 8, 40, 72, 68, 16, 24, 56, 88, 197, 32, 8, 40, 72, 70, 64, 24, 56, 88, 199, 128, 8, 40, 72, 72, 1, 25, 57, 89, 201, 2, 9, 41, 73, 74, 4, 25, 57, 89, 203, 8, 9, 41, 73, 76, 16, 25, 57, 89, 205, 32, 9, 41, 73, 78, 64, 25, 57, 89, 
207, 128, 9, 41, 73, 80, 1, 26, 58, 90, 209, 2, 10, 42, 74, 82, 4, 26, 58, 90, 211, 8, 10, 42, 74, 84, 16, 26, 58, 90, 213, 32, 10, 42, 74, 86, 64, 26, 58, 90, 215, 128, 10, 42, 74, 88, 1, 27, 59, 91, 217, 2, 11, 43, 75, 90, 4, 27, 59, 91, 219, 8, 11, 43, 75, 92, 16, 27, 59, 91, 221, 32, 11, 43, 75, 94, 64, 27, 59, 91, 
223, 128, 11, 43, 75, 96, 1, 28, 60, 92, 225, 2, 12, 44, 76, 98, 4, 28, 60, 92, 227, 8, 12, 44, 76, 100, 16, 28, 60, 92, 229, 32, 12, 44, 76, 102, 64, 28, 60, 92, 231, 128, 12, 44, 76, 104, 1, 29, 61, 93, 233, 2, 13, 45, 77, 106, 4, 29, 61, 93, 235, 8, 13, 45, 77, 108, 16, 29, 61, 93, 237, 32, 13, 45, 77, 110, 64, 29, 61, 93, 
239, 128, 13, 45, 77, 112, 1, 30, 62, 94, 241, 2, 14, 46, 78, 114, 4, 30, 62, 94, 243, 8, 14, 46, 78, 116, 16, 30, 62, 94, 245, 32, 14, 46, 78, 118, 64, 30, 62, 94, 247, 128, 14, 46, 78, 120, 1, 31, 63, 95, 249, 2, 15, 47, 79, 122, 4, 31, 63, 95, 251, 8, 15, 47, 79, 124, 16, 31, 63, 95, 253, 32, 15, 47, 79, 126, 64, 31, 63, 95, 
255, 128, 15, 47, 79, 128, 1, 0, 32, 64, 1, 2, 16, 48, 80, 130, 4, 0, 32, 64, 3, 8, 16, 48, 80, 132, 16, 0, 32, 64, 5, 32, 16, 48, 80, 134, 64, 0, 32, 64, 7, 128, 16, 48, 80, 136, 1, 1, 33, 65, 9, 2, 17, 49, 81, 138, 4, 1, 33, 65, 11, 8, 17, 49, 81, 140, 16, 1, 33, 65, 13, 32, 17, 49, 81, 142, 64, 1, 33, 65, 
15, 128, 17, 49, 81, 144, 1, 2, 34, 66, 17, 2, 18, 50, 82, 146, 4, 2, 34, 66, 19, 8, 18, 50, 82, 148, 16, 2, 34, 66, 21, 32, 18, 50, 82, 150, 64, 2, 34, 66, 23, 128, 18, 50, 82, 152, 1, 3, 35, 67, 25, 2, 19, 51, 83, 154, 4, 3, 35, 67, 27, 8, 19, 51, 83, 156, 16, 3, 35, 67, 29, 32, 19, 51, 83, 158, 64, 3, 35, 67, 
31, 128, 19, 51, 83, 160, 1, 4, 36, 68, 33, 2, 20, 52, 84, 162, 4, 4, 36, 68, 35, 8, 20, 52, 84, 164, 16, 4, 36, 68, 37, 32, 20, 52, 84, 166, 64, 4, 36, 68, 39, 128, 20, 52, 84, 168, 1, 5, 37, 69, 41, 2, 21, 53, 85, 170, 4, 5, 37, 69, 43, 8, 21, 53, 85, 172, 16, 5, 37, 69, 45, 32, 21, 53, 85, 174, 64, 5, 37, 69, 
47, 128, 21, 53, 85, 176, 1, 6, 38, 70, 49, 2, 22, 54, 86, 178, 4, 6, 38, 70, 51, 8, 22, 54, 86, 180, 16, 6, 38, 70, 53, 32, 22, 54, 86, 182, 64, 6, 38, 70, 55, 128, 22, 54, 86, 184, 1, 7, 39, 71, 57, 2, 23, 55, 87, 186, 4, 7, 39, 71, 59, 8, 23, 55, 87, 188, 16, 7, 39, 71, 61, 32, 23, 55, 87, 190, 64, 7, 39, 71, 
63, 128, 23, 55, 87, 192, 1, 8, 40, 72, 65, 2, 24, 56, 88, 194, 4, 8, 40, 72, 67, 8, 24, 56, 88, 196, 16, 8, 40, 72, 69, 32, 24, 56, 88, 198, 64, 8, 40, 72, 71, 128, 24, 56, 88, 200, 1, 9, 41, 73, 73, 2, 25, 57, 89, 202, 4, 9, 41, 73, 75, 8, 25, 57, 89, 204, 16, 9, 41, 73, 77, 32, 25, 57, 89, 206, 64, 9, 41, 73, 
79, 128, 25, 57, 89, 208, 1, 10, 42, 74, 81, 2, 26, 58, 90, 210, 4, 10, 42, 74, 83, 8, 26, 58, 90, 212, 16, 10, 42, 74, 85, 32, 26, 58, 90, 214, 64, 10, 42, 74, 87, 128, 26, 58, 90, 216, 1, 11, 43, 75, 89, 2, 27, 59, 91, 218, 4, 11, 43, 75, 91, 8, 27, 59, 91, 220, 16, 11, 43, 75, 93, 32, 27, 59, 91, 222, 64, 11, 43, 75, 
95, 128, 27, 59, 91, 224, 1, 12, 44, 76, 97, 2, 28, 60, 92, 226, 4, 12, 44, 76, 99, 8, 28, 60, 92, 228, 16, 12, 44, 76, 101, 32, 28, 60, 92, 230, 64, 12, 44, 76, 103, 128, 28, 60, 92, 232, 1, 13, 45, 77, 105, 2, 29, 61, 93, 234, 4, 13, 45, 77, 107, 8, 29, 61, 93, 236, 16, 13, 45, 77, 109, 32, 29, 61, 93, 238, 64, 13, 45, 77, 
111, 128, 29, 61, 93, 240, 1, 14, 46, 78, 113, 2, 30, 62, 94, 242, 4, 14, 46, 78, 115, 8, 30, 62, 94, 244, 16, 14, 46, 78, 117, 32, 30, 62, 94, 246, 64, 14, 46, 78, 119, 128, 30, 62, 94, 248, 1, 15, 47, 79, 121, 2, 31, 63, 95, 250, 4, 15, 47, 79, 123, 8, 31, 63, 95, 252, 16, 15, 47, 79, 125, 32, 31, 63, 95, 254, 64, 15, 47, 79, 
127, 128, 31, 63, 95, 
};

//----------------------------------------------------------------------------------------------------------------------
const static byte addressing_green[Constants::FrameSize * 5] = { 
0, 2, 112, 144, 176, 1, 1, 112, 144, 176, 2, 4, 112, 144, 176, 3, 8, 112, 144, 176, 4, 16, 112, 144, 176, 5, 32, 112, 144, 176, 6, 64, 112, 144, 176, 7, 128, 112, 144, 176, 8, 1, 113, 145, 177, 9, 2, 113, 145, 177, 10, 4, 113, 145, 177, 11, 8, 113, 145, 177, 12, 16, 113, 145, 177, 13, 32, 113, 145, 177, 14, 64, 113, 145, 177, 
15, 128, 113, 145, 177, 16, 2, 114, 146, 178, 17, 1, 114, 146, 178, 18, 4, 114, 146, 178, 19, 8, 114, 146, 178, 20, 16, 114, 146, 178, 21, 32, 114, 146, 178, 22, 64, 114, 146, 178, 23, 128, 114, 146, 178, 24, 1, 115, 147, 179, 25, 2, 115, 147, 179, 26, 4, 115, 147, 179, 27, 8, 115, 147, 179, 28, 16, 115, 147, 179, 29, 32, 115, 147, 179, 30, 64, 115, 147, 179, 
31, 128, 115, 147, 179, 32, 2, 116, 148, 180, 33, 1, 116, 148, 180, 34, 4, 116, 148, 180, 35, 8, 116, 148, 180, 36, 16, 116, 148, 180, 37, 32, 116, 148, 180, 38, 64, 116, 148, 180, 39, 128, 116, 148, 180, 40, 1, 117, 149, 181, 41, 2, 117, 149, 181, 42, 4, 117, 149, 181, 43, 8, 117, 149, 181, 44, 16, 117, 149, 181, 45, 32, 117, 149, 181, 46, 64, 117, 149, 181, 
47, 128, 117, 149, 181, 48, 2, 118, 150, 182, 49, 1, 118, 150, 182, 50, 4, 118, 150, 182, 51, 8, 118, 150, 182, 52, 16, 118, 150, 182, 53, 32, 118, 150, 182, 54, 64, 118, 150, 182, 55, 128, 118, 150, 182, 56, 1, 119, 151, 183, 57, 2, 119, 151, 183, 58, 4, 119, 151, 183, 59, 8, 119, 151, 183, 60, 16, 119, 151, 183, 61, 32, 119, 151, 183, 62, 64, 119, 151, 183, 
63, 128, 119, 151, 183, 64, 2, 120, 152, 184, 65, 1, 120, 152, 184, 66, 4, 120, 152, 184, 67, 8, 120, 152, 184, 68, 16, 120, 152, 184, 69, 32, 120, 152, 184, 70, 64, 120, 152, 184, 71, 128, 120, 152, 184, 72, 1, 121, 153, 185, 73, 2, 121, 153, 185, 74, 4, 121, 153, 185, 75, 8, 121, 153, 185, 76, 16, 121, 153, 185, 77, 32, 121, 153, 185, 78, 64, 121, 153, 185, 
79, 128, 121, 153, 185, 80, 2, 122, 154, 186, 81, 1, 122, 154, 186, 82, 4, 122, 154, 186, 83, 8, 122, 154, 186, 84, 16, 122, 154, 186, 85, 32, 122, 154, 186, 86, 64, 122, 154, 186, 87, 128, 122, 154, 186, 88, 1, 123, 155, 187, 89, 2, 123, 155, 187, 90, 4, 123, 155, 187, 91, 8, 123, 155, 187, 92, 16, 123, 155, 187, 93, 32, 123, 155, 187, 94, 64, 123, 155, 187, 
95, 128, 123, 155, 187, 96, 2, 124, 156, 188, 97, 1, 124, 156, 188, 98, 4, 124, 156, 188, 99, 8, 124, 156, 188, 100, 16, 124, 156, 188, 101, 32, 124, 156, 188, 102, 64, 124, 156, 188, 103, 128, 124, 156, 188, 104, 1, 125, 157, 189, 105, 2, 125, 157, 189, 106, 4, 125, 157, 189, 107, 8, 125, 157, 189, 108, 16, 125, 157, 189, 109, 32, 125, 157, 189, 110, 64, 125, 157, 189, 
111, 128, 125, 157, 189, 112, 2, 126, 158, 190, 113, 1, 126, 158, 190, 114, 4, 126, 158, 190, 115, 8, 126, 158, 190, 116, 16, 126, 158, 190, 117, 32, 126, 158, 190, 118, 64, 126, 158, 190, 119, 128, 126, 158, 190, 120, 1, 127, 159, 191, 121, 2, 127, 159, 191, 122, 4, 127, 159, 191, 123, 8, 127, 159, 191, 124, 16, 127, 159, 191, 125, 32, 127, 159, 191, 126, 64, 127, 159, 191, 
127, 128, 127, 159, 191, 128, 2, 96, 128, 160, 129, 1, 96, 128, 160, 130, 4, 96, 128, 160, 131, 8, 96, 128, 160, 132, 16, 96, 128, 160, 133, 32, 96, 128, 160, 134, 64, 96, 128, 160, 135, 128, 96, 128, 160, 136, 1, 97, 129, 161, 137, 2, 97, 129, 161, 138, 4, 97, 129, 161, 139, 8, 97, 129, 161, 140, 16, 97, 129, 161, 141, 32, 97, 129, 161, 142, 64, 97, 129, 161, 
143, 128, 97, 129, 161, 144, 2, 98, 130, 162, 145, 1, 98, 130, 162, 146, 4, 98, 130, 162, 147, 8, 98, 130, 162, 148, 16, 98, 130, 162, 149, 32, 98, 130, 162, 150, 64, 98, 130, 162, 151, 128, 98, 130, 162, 152, 1, 99, 131, 163, 153, 2, 99, 131, 163, 154, 4, 99, 131, 163, 155, 8, 99, 131, 163, 156, 16, 99, 131, 163, 157, 32, 99, 131, 163, 158, 64, 99, 131, 163, 
159, 128, 99, 131, 163, 160, 2, 100, 132, 164, 161, 1, 100, 132, 164, 162, 4, 100, 132, 164, 163, 8, 100, 132, 164, 164, 16, 100, 132, 164, 165, 32, 100, 132, 164, 166, 64, 100, 132, 164, 167, 128, 100, 132, 164, 168, 1, 101, 133, 165, 169, 2, 101, 133, 165, 170, 4, 101, 133, 165, 171, 8, 101, 133, 165, 172, 16, 101, 133, 165, 173, 32, 101, 133, 165, 174, 64, 101, 133, 165, 
175, 128, 101, 133, 165, 176, 2, 102, 134, 166, 177, 1, 102, 134, 166, 178, 4, 102, 134, 166, 179, 8, 102, 134, 166, 180, 16, 102, 134, 166, 181, 32, 102, 134, 166, 182, 64, 102, 134, 166, 183, 128, 102, 134, 166, 184, 1, 103, 135, 167, 185, 2, 103, 135, 167, 186, 4, 103, 135, 167, 187, 8, 103, 135, 167, 188, 16, 103, 135, 167, 189, 32, 103, 135, 167, 190, 64, 103, 135, 167, 
191, 128, 103, 135, 167, 192, 2, 104, 136, 168, 193, 1, 104, 136, 168, 194, 4, 104, 136, 168, 195, 8, 104, 136, 168, 196, 16, 104, 136, 168, 197, 32, 104, 136, 168, 198, 64, 104, 136, 168, 199, 128, 104, 136, 168, 200, 1, 105, 137, 169, 201, 2, 105, 137, 169, 202, 4, 105, 137, 169, 203, 8, 105, 137, 169, 204, 16, 105, 137, 169, 205, 32, 105, 137, 169, 206, 64, 105, 137, 169, 
207, 128, 105, 137, 169, 208, 2, 106, 138, 170, 209, 1, 106, 138, 170, 210, 4, 106, 138, 170, 211, 8, 106, 138, 170, 212, 16, 106, 138, 170, 213, 32, 106, 138, 170, 214, 64, 106, 138, 170, 215, 128, 106, 138, 170, 216, 1, 107, 139, 171, 217, 2, 107, 139, 171, 218, 4, 107, 139, 171, 219, 8, 107, 139, 171, 220, 16, 107, 139, 171, 221, 32, 107, 139, 171, 222, 64, 107, 139, 171, 
223, 128, 107, 139, 171, 224, 2, 108, 140, 172, 225, 1, 108, 140, 172, 226, 4, 108, 140, 172, 227, 8, 108, 140, 172, 228, 16, 108, 140, 172, 229, 32, 108, 140, 172, 230, 64, 108, 140, 172, 231, 128, 108, 140, 172, 232, 1, 109, 141, 173, 233, 2, 109, 141, 173, 234, 4, 109, 141, 173, 235, 8, 109, 141, 173, 236, 16, 109, 141, 173, 237, 32, 109, 141, 173, 238, 64, 109, 141, 173, 
239, 128, 109, 141, 173, 240, 2, 110, 142, 174, 241, 1, 110, 142, 174, 242, 4, 110, 142, 174, 243, 8, 110, 142, 174, 244, 16, 110, 142, 174, 245, 32, 110, 142, 174, 246, 64, 110, 142, 174, 247, 128, 110, 142, 174, 248, 1, 111, 143, 175, 249, 2, 111, 143, 175, 250, 4, 111, 143, 175, 251, 8, 111, 143, 175, 252, 16, 111, 143, 175, 253, 32, 111, 143, 175, 254, 64, 111, 143, 175, 
255, 128, 111, 143, 175, 
};

//----------------------------------------------------------------------------------------------------------------------
void loadFrame(byte* buffer, byte* frame)
{
  int adrG = 0, adrR = 0;
  for(int i = 0; i < Constants::FrameSize; ++i)
  { 
    byte pixel_r = frame[addressing_red[adrR ++]];
    byte pixel_g = frame[addressing_green[adrG ++]];
    
    byte red   = pixel_r & 0x0f;
    byte green = (pixel_g >> 4) & 0x0f;

    // some swapping stuff because the hardware is wired upside-down :-)
    int offset = Constants::FrameSize / 2;
    if(i >= offset)
      i -= offset;
    else
      i += offset;


    // red
    // ------------------------------------------------
    byte mask = addressing_red[adrR ++];
    
    for(int j = 0; j < red && j < 3; ++j)
    {
      buffer[ addressing_red[adrR + j] ] |= mask;
    }

    adrR += 3;
    

    // green
    // ------------------------------------------------
    mask = addressing_green[adrG ++];
    
    for(int j = 0; j < green && j < 3; ++j)
    {
      buffer[ addressing_green[adrG + j] ] |= mask;
    } 
    
    adrG += 3;
  }
}
