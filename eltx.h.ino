//import necessary libraries
#include <SPI.h>
#include "nrf24l01.h"
#include "RF24.h"

//comment this out to prevent Serial from being used.
//unnecessary processing for the final product
#define DEBUG

struct {
  int pot_val;
  bool c_button;
  bool z_button;
}  rfData;

