#include "CANCommunication.h"

#define TRANSMIT_ID 0x300
#define RECEIVE_ID 0x200

CANCommunication* CanCom;
byte receivedNumber;

void setup()
{
  Serial.begin(115200);
  CanCom = new CANCommunication(TRANSMIT_ID, RECEIVE_ID);
  CanCom->CANBusInit();

  Serial.println("Node Ready!");
}

void loop()
{
    //--------------------------------------
    // SEND A RANDOM NUMBER EVERY 5 SECONDS
    //--------------------------------------

    CanCom->CANBusSendMsg();

    //--------------------------------------
    //          RECEIVE MESSAGES
    //--------------------------------------

    CanCom->CANBusReceiveMsg(&receivedNumber);
}





