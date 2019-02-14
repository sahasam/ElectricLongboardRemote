/* 
 * 
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

//PIN CONSTANTS
#define ESC_PIN  6 //pin where esc data wire is attached

//BOUNDARY CONSTANTS
#define MIN_ESC_PULSE     1000
#define MAX_ESC_PULSE     2000
#define CC_INCREMENT         2


//This is the data struct that should be sent over rf
struct {
  int pot_val;
  bool c_button;
  bool z_button;
}  rfData;


//setup pipe addresses.
//TX and RX will be reversed so that the writing pipe of one
//is the reading pipe of the other. Acknowledges are buggy on
//my setup, so the reading pipe of TX and the writing pipe of RX
//are not used.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


#endif


