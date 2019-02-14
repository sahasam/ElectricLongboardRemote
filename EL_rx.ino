/* Copyright (C) 2018 Sahas Munamala <munamalasahas@gmail.com>

    This program is the receiver in the tx/rx couple for the
    electric longboard project.

    Connect the nrf24l01+'s CE and CSN pins to pins 9 and 10
      respectively.
    Connect the ESC data pin to

    The General Overview:
      1. gather data about joystick and buttons into a struct
         that is known by noth the tx and the rx
      2. send the data as fast as possible. Whatever sticks and
         is good data will be used by the RX.

    Note:
      Only raw data will be transmitted over RF. If transmission speed
      becomes a problem, you can convert the raw data to servo angles
      using the logic on the RX side. Then send the servo angle.
*/
//////////////////////////////////////////////////////////////////////////////
//import necessary libraries
#include <SPI.h>
#include <Servo.h>
#include "nRF24L01.h"
#include "printf.h"
#include "RF24.h"

#include "constants.h"

//comment this out to prevent Serial from being used.
//unnecessary processing for the final product
#define DEBUG
//////////////////////////////////////////////////////////////////////////////

//set up RF24 on SPI bus plus 9 & 10 for CE/CSN
RF24 radio(9, 10);

//Create the esc object which is represented by a Servo because
//the esc uses servo signals to set speed
Servo esc;

bool _forwards = true;
bool _cc_enabled = false;
int _cur_signal = 0;

void setup()
{
#ifdef DEBUG
  Serial.begin(57600);
  Serial.println("EL_RX started");
  printf_begin();
#endif

  /*begin the radio and establish all the settings*/
  //begin radio
  radio.begin();

  //turn off acknoweldges which are buggy with my nRFs
  radio.setAutoAck(false);

  //set size of payload to minimum necessary bytes
  radio.setPayloadSize( sizeof(rfData) );

  //establish reading and writing pipes for the TX.
  //Keep in mind, these are crossed on the RX side.
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);

  //just to make sure it can read.
  radio.startListening();

  //initialize the esc object by attaching it to the
  //necessary pins.
  esc.attach(ESC_PIN);

  //print valuable debug information to console.
#ifdef DEBUG
  radio.printDetails();
#endif
}


/*
   Main loop.
    1. gets data from transmitter
    2. writes the proper data to receiver.
    3. safety systems in case transmission times out
    4. repeat
*/
void loop()
{

  //////////////////////////////////
  //Step 1: GET DATA FROM RECEIVER//
  //////////////////////////////////
  bool done = false;
  if (radio.available())
  {
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &rfData, sizeof(rfData) );

#ifdef DEBUG
      //Serial.println(String(rfData.pot_val) + ", " + String(rfData.c_button) + ", " + String(rfData.z_button));
      delay(20);
#endif

      ///////////////////////////////////
      //Step 2. Write data to receiver //
      ///////////////////////////////////
      //NOTE: ESC will be written to even if data is not being read.
      //      This is to allow safety features regarded signal dropout
      //      to be addressed.
      if (!_cc_enabled)
      {
        setMSpeed(rfData.pot_val);
      }
      else
      {
        setMSpeed(cruiseControl(rfData.pot_val));
      }

#ifdef DEBUG
      //Serial.println("_cur_signal: " + String(rfData.pot_val) );
#endif

      if (rfData.c_button)
        _cc_enabled = true;
      else
        _cc_enabled = false;
    }
  }
}

/*
   Handles the setting of the motorspeed going forwards or backwards.

   Parameters:  int pot_val   - the potentiometer value from the rfData struct

   Function:
    if going forwards, set the value directly directly mapped to 1 ms - 2 ms (default)
    if going backwards, subtract from MAX_ESC_PULSE, and add MIN_ESC_PULSE to
      inverse the direction

*/
void setMSpeed (int pot_val)
{
  if (_forwards)
  {
    int mappedVal = map(pot_val, 0, 1023, MIN_ESC_PULSE, MAX_ESC_PULSE);
    esc.writeMicroseconds( mappedVal );
    if (!_cc_enabled)
      _cur_signal = pot_val;
    Serial.println("cur_signal: " + String(_cur_signal));
  }
  else
  {
    int mappedVal = MIN_ESC_PULSE +
                    (MAX_ESC_PULSE - map(pot_val, 0, 1023, MIN_ESC_PULSE, MAX_ESC_PULSE) );
    esc.writeMicroseconds( mappedVal );
    _cur_signal = pot_val;
  }
}

/*
   Control function that handles the movement if the longboard is in cruise control mode

   Parameters:  int pot_val   - the potentiometer value from the rfData struct
                bool forwards - the direction at which the car should be going.
                int cur_speed - the current servo signal (us) being sent out
                                (between 1000 and 2000)

   Returns:     the speed the car should be going at after the decrement.
                decrements 1 us at the minimum, and 3 us at the maximum.
                change the scale by editing the CC_MIN_INCR_DECR and
                CC_MAX_INCR_DECR constants in constants.h

   Function:
     Establish values < 480 and > 580 to be actionable, and respond accordingly
     (neutral rests around 530 for my potentiometer)
        1. if below 480, return a speed lower than _cur_signal
        2. if greater than 580, return a speed higher than _cur_signal

        make sure increments and decrements are proportinal to deviation from
        neutral position
*/
int cruiseControl (int pot_val)
{
  if (pot_val <= 480)
  {
    int increment = 2;
    if (_cur_signal - increment >= 500)
    {
      _cur_signal -= increment;
      return _cur_signal;
    }
    else
    {
      return _cur_signal;
    }
  }
  else if (pot_val >= 580)
  {
    int increment = 2;

    if (_cur_signal + increment <= 1023)
    {
      _cur_signal += increment;
      return _cur_signal;
    }
    else
    {
      return _cur_signal;
    }
  }
}

