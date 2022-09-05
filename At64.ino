/*
 * Arduino N64 Controller
 * Nino MegaDriver - nino@nino.com.br
 * http://www.megadriver.com.br
 * 
 * License: GPL 3.0 Mixed
 * 
 * Arduino sketch to turn an AtMega328 board into
 * a N64 Controller.
 * 
 * IMPORTANT: The N64 operates at 3.3v and Arduino
 *            at 5v. So YOU MUST use a bidirectional
 *            logic level converter to connect
 *            the Arduino to N64. Dont't connect the
 *            arduino directly to the N64 console or
 *            you'll damage it.
 * 
 * Console side connection:
 *  (facing the console)
 * 
 *    /-------------\
 *   / O    O     O  \
 *  | 3.3V Signal GND |
 *  |_________________|
 *  
 *  3.3v   => Logic level converter "low" vcc
 *  Signal => Logic level converter "low" input
 *  GND    => Logic level converter "low" ground
 *  
 *  Arduino side connection:
 *  
 *  GND    => Logic level converter "high" ground
 *  +5     => Logic level converter "high" vcc
 *  Pin 2  => Logic level converter "high" input
 *  
 *  Input pins [pulled high]:
 *  
 *  [Arduino] [N64 Button]
 *  3      => START
 *  4      => Up
 *  5      => Down
 *  6      => Left
 *  7      => Right
 *  8      => Button B
 *  9      => C Left
 *  10     => C Up
 *  11     => Button A
 *  12     => C Right
 *  13     => C Down
 *  A0     => Button Z
 *  A1     => Left Shoulder
 *  A2     => Right Shoulder
 *  
 *  Directional inputs configurations:
 *  
 *  A3     => If connected to ground, directional axis are interpreted
 *            as encoders. With this setup is possible to use a wheel
 *            and a gas pedal, for example, on driving games.
 *            Connect the A/B pins of the encoder to pins 4 and 5 for the Y
 *            axis and 6 and 7 for the X axis.
 *            
 *  A5     => If connected to ground, directional is routed to the analog stick
 *            If left as is (HIGH), it's routed to digital pad. Some games
 *            like Killer Instinct Gold and Mortal Kombat Trilogy uses the digital
 *            pad, others like Smash Bros uses the analog stick.
 *            
 */

// Uses RotaryEncoder library
// Install it on Sketch->Include Library->Manage Libraries
#include <RotaryEncoder.h>
#include "n64.h"

RotaryEncoder *Xaxis, *Yaxis; // Encoder object pointers
uint8_t encoder_loaded = 0; // Keep track of Encoder objects
int N64X=0, N64Y=0; // Current value of the axis read from the Encoders

// N64 controller buttons bits
// PORT A
#define BTN_A  0b10000000
#define BTN_B  0b01000000
#define BTN_Z  0b00100000
#define START  0b00010000
#define DUP    0b00001000
#define DDOWN  0b00000100
#define DLEFT  0b00000010
#define DRIGHT 0b00000001
// PORT B
#define BTN_L  0b00100000
#define BTN_R  0b00010000
#define CUP    0b00001000
#define CDOWN  0b00000100
#define CLEFT  0b00000010
#define CRIGHT 0b00000001

// N64 controller analog stick maximum
// and minimum values.
// Ranges from -84 to +84
// measured from a real and original
// N64 controller.
#define AUP    0x54 // Analog Y AXIS +84
#define ADOWN  0xAC // Analog Y AXIS -84
#define ALEFT  0xAC // Analog X AXIS +84
#define ARIGHT 0x54 // Analog X AXIS -84

// Configure pins
void pinSetup(){
  for(int i=3; i<=13;i++) pinMode(i, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

void setup() {
  pinSetup();
}

void loop() {


  uint8_t *rbuf; // Input Buffer
  uint8_t ibuf[] = {0x05, 0x00, 0x00}; // Interface buffer, controller ID

  //                Controller status buffer - 8 bytes
  //                [PORTA] [PORTB] [X AXIS] [YAXIS]
  uint8_t sbuf[] = { 0x00,   0x00,   0x00,    0x00,  0x00, 0x00, 0x00, 0x00 };
  if(digitalRead(A0) == LOW)   sbuf[1] |= BTN_L;
  if(digitalRead(A1) == LOW)   sbuf[1] |= BTN_R;
  if(digitalRead(A2) == LOW)   sbuf[0] |= BTN_Z;
  if(digitalRead(3)  == LOW)   sbuf[0] |= START;

  // DIRECTIONAL DECODING
  // Are we using Encoders or joystics?
  if(digitalRead(A3)  == LOW){ // We're using encoders
    if(encoder_loaded == 0){ // Create new Encoder instances
      Yaxis = new RotaryEncoder(4,5, RotaryEncoder::LatchMode::FOUR3);
      Xaxis = new RotaryEncoder(6, 7, RotaryEncoder::LatchMode::FOUR3);
      encoder_loaded = 1;
    }
     Xaxis->tick();
     int dir = (int)(Xaxis->getDirection());
     if(dir > 0) {
      N64X++;
      if(N64X > 84) N64X = 84;
     }else if(dir < 0){
      N64X--;
      if(N64X < -84) N64X = -84;
     }
     sbuf[2] = N64X;
     Yaxis->tick();
     dir = (int)(Yaxis->getDirection());
     if(dir > 0) {
      N64Y++;
      if(N64Y > 84) N64Y = 84;
     }else if(dir < 0){
      N64Y--;
      if(N64Y < -84) N64Y = -84;
     }
     sbuf[3] = N64Y;     
  }else{ // We're using joystics
    if(encoder_loaded == 1){ // If we did load Encoders in the past, destroy'em!
      delete Xaxis;
      delete Yaxis;
      encoder_loaded = 0;
      pinSetup(); // Reconfigure the pins
    }
    if(digitalRead(A5)  == HIGH) { // Using digital buttons *default*
      if(digitalRead(4)  == LOW) sbuf[0] |= DUP;
      if(digitalRead(5)  == LOW) sbuf[0] |= DDOWN;
      if(digitalRead(6)  == LOW) sbuf[0] |= DLEFT;
      if(digitalRead(7)  == LOW) sbuf[0] |= DRIGHT;
    }else{  // Using Analog stick
      if(digitalRead(4)  == LOW) sbuf[3]  = AUP; 
      if(digitalRead(5)  == LOW) sbuf[3]  = ADOWN;
      if(digitalRead(6)  == LOW) sbuf[2]  = ALEFT;
      if(digitalRead(7)  == LOW) sbuf[2]  = ARIGHT;
    }
  }
  
  // BUTTONS DECODING
  if(digitalRead(8)  == LOW)   sbuf[0] |= BTN_B;
  if(digitalRead(9)  == LOW)   sbuf[1] |= CLEFT;
  if(digitalRead(10) == LOW)   sbuf[1] |= CUP;
  if(digitalRead(11) == LOW)   sbuf[0] |= BTN_A;
  if(digitalRead(12) == LOW)   sbuf[1] |= CDOWN;
  if(digitalRead(13) == LOW)   sbuf[1] |= CRIGHT;

  uint8_t oldSREG = SREG;
  cli();                                                                                       // Clear interrupts for a safer run.
  n64_get(                     rbuf, 3, 0x2a, 0x2b, 0x29, 0x04); // Get command from N64 console.
  if(rbuf[0] == 0x00) n64_send(ibuf, 3, 0x2a, 0x2b, 0x04);                 // Command 0x00: Console requesting controller ID, reply with it.
  if(rbuf[0] == 0x01) n64_send(sbuf, 8, 0x2a, 0x2b, 0x04);                 // Command 0x01: Console requesting controller status, reply with it.
  SREG = oldSREG;
  
}
