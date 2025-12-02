#include <mcp_can.h>
#include <SPI.h>
#include "CANCommunication.h"

byte *receivedNumber;

void setup()
{
  Serial.begin(115200);
  CANBusInit();

  Serial.println("Node Ready!");
}

void loop()
{
    //--------------------------------------
    // SEND A RANDOM NUMBER EVERY 5 SECONDS
    //--------------------------------------

    CANBusSendMsg();

    //--------------------------------------
    //          RECEIVE MESSAGES
    //--------------------------------------

    CANBusReceiveMsg(receivedNumber);
}





