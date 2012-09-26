#include <stdlib.h>
#include <unistd.h>
#include "oddity.h"

extern "C"
{
int _getpid( void ) { return 0; }
void _exit(int status) {}
int _kill(pid_t pid, int sig) { return 0; }
}

HardwareSPI spi(1);

HardwareTimer displayTimer(1);
HardwareTimer frameTimer(2);

// column data pins
int LE = 9;
int OE = 8;

// column driver pins
int CLK = 7;
int DATA = 6;
int STROBE = 5;

#define FRAME_TIME 50


//
// frame buffer stored as 6 bitmaps (2 colors x 3 color
// intensities) ordered: R0 R1 R2 G0 G1 G2
//
// 32 bytes per pixel per color
#define BITMAP_SIZE  (FRAME_WIDTH * FRAME_HEIGHT / 8)
#define BUFFER_SIZE  (BITMAP_SIZE * 6)
#define GREEN_OFFSET (BITMAP_SIZE * 3)

//
// variables for display update
int gIntensity;
int gColumn;
int gIntensityMap[] = { 50, 100, 150 }; // geometric 50, 150, 300

//
// buffer (double)
void clearFrame(byte* frame);
void clearBuffer(byte* buffer);

byte buffer[BUFFER_SIZE * 2];
void flipBuffer();
byte* pFrontBuffer;
byte* pBackBuffer;

//
// frame
void loadFrame(byte* buffer, byte* frame);
volatile boolean gHaveNewFrame;
byte frame[FRAME_SIZE];

//
// Serial stuff
char gSerialBuffer[32];
int gSerialIndex;

//
// Encoder inputs!
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

//
// main code
//----------------------------------------------------------------------------------------------------------------------
void setup()
{
  initialize();
  
  clearFrame(frame);
  loadFrame(pBackBuffer, frame);
  flipBuffer();
  
  oddity_init();
}

void loop()
{
  if (gHaveNewFrame)
    return;
  
  int alA = analogIncA;
  int alB = analogIncB;
  int alC = analogIncC;
  
  hw_inputs inputs;
  inputs.encoderChange[0] = (alA - analogLastA);
  inputs.encoderChange[1] = (alB - analogLastB);
  inputs.encoderChange[2] = (alC - analogLastC);
  analogLastA = alA;
  analogLastB = alB;
  analogLastC = alC;
  
  
  clearFrame(frame);
  
  if (oddity_tick(frame, inputs))
  {
    clearBuffer(pBackBuffer);
    loadFrame(pBackBuffer, frame);
    gHaveNewFrame = true;
  }
}

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
  
  gSerialIndex = 0;
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
  //SerialUSB.println(analogIncA);
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

void clearFrame(byte* frame)
{
  for(int i = 0; i < FRAME_SIZE; ++i)
    frame[i] = 0;
}

void clearBuffer(byte* buffer)
{
  for(int i = 0; i < BUFFER_SIZE; ++i)
    buffer[i] = 0;
}

void flipBuffer()
{
  byte* temp = pFrontBuffer;
  pFrontBuffer = pBackBuffer;
  pBackBuffer = temp; 
}

void loadFrame(byte* buffer, byte* frame)
{
  for(int i = 0; i < FRAME_SIZE; ++i)
  { 
    byte pixel = frame[i];
    byte red   = pixel & 0x0f;
    byte green = (pixel >> 4) & 0x0f;

    // some swapping stuff because the hardware is wired upside-down :-)
    int offset = FRAME_SIZE / 2;
    if(i >= offset)
      i -= offset;
    else
      i+= offset;

    byte mask = (1 << (i % 8));

    // red
    for(int j = 0; j < red && j < 3; ++j)
    {
      buffer[j * BITMAP_SIZE + i / 8] |= mask;
    }

    for(int j = 0; j < green && j < 3; ++j)
    {
      buffer[GREEN_OFFSET + j * BITMAP_SIZE + i / 8] |= mask;
    } 
  }
}
