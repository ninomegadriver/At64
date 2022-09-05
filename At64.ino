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
 * IMPORTANT: Use a logic level converter to connect
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
 *  2      => Logic level converter "high" input
 *  
 *  Input pins [pulled high]:
 *  
 *  [Arduino] [N64 Button]
 *  3      => START
 *  4      => Up    (see A5 bellow)
 *  5      => Down  (see A5 bellow)
 *  6      => Left  (see A5 bellow)
 *  7      => Right (see A5 bellow)
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
 *  A5     => If connected to ground, directional routed to analog stick
 *            If left as is (HIGH), direction routed to digital pad
 *            
 *  TODO: Fix the code and get rid of all the warnings
 */

#include <Arduino.h>
#include "n64.h"

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

// N64 controller analog stick
// Ranges from -84 to +84
// measured from a real and original
// N64 controller.
#define AUP    0x54 // Analog Y AXIS +84
#define ADOWN  0xAC // Analog Y AXIS -84
#define ALEFT  0xAC // Analog X AXIS +84
#define ARIGHT 0x54 // Analog X AXIS -84

void setup() {
  for(int i=3; i<=13;i++) pinMode(i, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
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
  if(digitalRead(A5)  == HIGH) {
    if(digitalRead(4)  == LOW) sbuf[0] |= DUP;
    if(digitalRead(5)  == LOW) sbuf[0] |= DDOWN;
    if(digitalRead(6)  == LOW) sbuf[0] |= DLEFT;
    if(digitalRead(7)  == LOW) sbuf[0] |= DRIGHT;
  }else{
    if(digitalRead(4)  == LOW) sbuf[3]  = AUP;
    if(digitalRead(5)  == LOW) sbuf[3]  = ADOWN;
    if(digitalRead(6)  == LOW) sbuf[2]  = ALEFT;
    if(digitalRead(7)  == LOW) sbuf[2]  = ARIGHT;
  }
  if(digitalRead(8)  == LOW)   sbuf[0] |= BTN_B;
  if(digitalRead(9)  == LOW)   sbuf[1] |= CLEFT;
  if(digitalRead(10) == LOW)   sbuf[1] |= CUP;
  if(digitalRead(11) == LOW)   sbuf[0] |= BTN_A;
  if(digitalRead(12) == LOW)   sbuf[1] |= CDOWN;
  if(digitalRead(13) == LOW)   sbuf[1] |= CRIGHT;

  uint8_t oldSREG = SREG;
  cli();
  n64_get(rbuf, 3, 0x2a, 0x2b, 0x29, 0x04); // Get command from N64 console
  if(rbuf[0] == 0x00) n64_send(ibuf, 3, 0x2a, 0x2b, 0x04); // Command 0x00: Console requesting controller ID, reply with it.
  if(rbuf[0] == 0x01) n64_send(sbuf, 8, 0x2a, 0x2b, 0x04); // Command 0x01: Console requesting controller status, reply with it.
  SREG = oldSREG;
  
}
