/* Copyright (C) 2018 Sahas Munamala <munamalasahas@gmail.com>

    This program is the transmitter in the tx/rx couple for the
    electric longboard project.

    Connect the nrf24l01+'s CE and CSN pins to pins 9 and 10
      respectively.
    Connect the buttons to D7 and D8
    Connect the analog joystick's y-axis to A0

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
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//comment this out to prevent Serial from being used.
//This is sunnecessary processing for the final product
#define DEBUG
//////////////////////////////////////////////////////////////////////////////

//This data struct is the data packet that will be sent over RF.
struct {
  int pot_val;
  bool c_button;
  bool z_button;
}  rfData;

//set up RF24 on SPI bus plus 9 & 10 for CE/CSN
RF24 radio(9, 10);

//setup pipe addresses.
//TX and RX will be reversed so that the writing pipe of one
//is the reading pipe of the other. Acknowledges are buggy on
//my setup, so the reading pipe of TX and the writing pipe of RX
//are not used.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//define input pins for joystick and buttons
//change these values as you see fit. Keep in mind
//the button pins have to be on open digital pins,
//and the POT_PIN has to be on an analog pin.
const int POT_PIN = A0;
const int C_PIN = 7;
const int Z_PIN = 8;

#ifdef DEBUG
unsigned long txStartTime;
unsigned long txEndTime;
int transmissionCount = 0;
#endif

void setup()
{
#ifdef DEBUG
  Serial.begin(57600);
  Serial.println("EL_TX started");
  printf_begin();
#endif

  //establish digital pins as inputs
  pinMode(C_PIN, INPUT);
  pinMode(Z_PIN, INPUT);


  /*begin the radio and establish all the settings*/
  //begin radio
  radio.begin();

  //turn off acknoweldges which are buggy with my nRFs
  radio.setAutoAck(false);

  //set size of payload to minimum necessary bytes
  radio.setPayloadSize( sizeof(rfData) );

  //establish reading and writing pipes for the TX.
  //Keep in mind, these are crossed on the RX side.
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);

  //just to make sure it can write.
  radio.stopListening();

  //print valuable debug information to console.
#ifdef DEBUG
  radio.printDetails();
#endif
}

void loop()
{
  ////////////////////////////////
  //STEP 1: INPUT DATA TO RFDATA
  ////////////////////////////////
  rfData.pot_val = analogRead(POT_PIN);
  rfData.c_button = digitalRead(C_PIN);
  rfData.z_button = digitalRead(Z_PIN);

  ////////////////////////////////
  //STEP 2. OUPUT DATA OVER RF
  ////////////////////////////////

  //If Debugging, keep track of start time and end time, then use them
  //to calculate the approximate transmission speed in Hz.
  //1000/time gives a quick way to estimate Hz. Not 100% accurate, close enough
#ifdef DEBUG
  txStartTime = millis();
  transmissionCount++;
#endif

  //Finally send the Data over RF
  if (radio.write ( &rfData, sizeof(rfData) ))
  {
    
#ifdef DEBUG
    txEndTime = millis();
    if ((transmissionCount % 100) == 0)
    {
      //Serial.println("..ok.. Sending at: ~" + String(1000.0 / (txEndTime - txStartTime)) + "Hz");
      Serial.println(rfData.pot_val);
      transmissionCount = 0;
    }
    transmissionCount++;
#endif

    delay(20);
  }
  radio.write( &rfData, sizeof(rfData) );


}
