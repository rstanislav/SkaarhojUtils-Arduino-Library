/*****************
 * This example return events from the encoders on SKAARHOJ Rotary Encoder Breakout Board
 * If the value is 0 (zero), nothing happened on the encoder
 * Values 1 and -1 indicates a rotation clockwise and counter-clockwise
 * Value 2 indicates a button press (on down)
 * Values larger than 2 indicates the release time of a button
 * If the second parameter (milliseconds) is sent when asking for state of the button, it indicates the time
 * when an automated release-event is detected on the buttons. 
 * 
 * - kasper
 *
 * This example code is in the public domain.
 */


#include "SkaarhojUtils.h"
SkaarhojUtils utils;

void setup() {
  Serial.begin(9600);
  Serial.println("Serial started");
  
  utils.encoders_init();
}

void loop() {
  int encValue = utils.encoders_state(0,1000);
  if (encValue)  {
     Serial.print("Encoder 0: ");
     Serial.println(encValue); 
  }

  encValue = utils.encoders_state(1,1000);
  if (encValue)  {
     Serial.print("Encoder 1: ");
     Serial.println(encValue); 
  }
}
