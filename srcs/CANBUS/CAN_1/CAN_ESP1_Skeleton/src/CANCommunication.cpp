#include "CANCommunication.h"

CANCommunication::CANCommunication(unsigned long TRANSMIT_ID, unsigned long RECEIVE_ID)
{
    this->CANBusObjectPointer = new MCP_CAN(5);
    this->TRANSMIT_ID = TRANSMIT_ID;
    this->RECEIVE_ID = RECEIVE_ID;
};

void CANCommunication::CANBusInit()
{
    SPI.begin(ESP_CLK_PIN, ESP_MISO_PIN, ESP_MOSI_PIN, CAN_CS_PIN);

  if (CANBusObjectPointer->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
  {
    Serial.println("MCP2515 Initialized Successfully!");
  }
  else
  {
    Serial.println("Error Initializing MCP2515...");
  }

  CANBusObjectPointer->setMode(MCP_NORMAL);
}

void CANCommunication::CANBusSendMsg()
{
    if (millis() - lastSendTime >= 5000)
    {
        byte randomNumber = random(1,41); // generate test value
        byte sndStat = CANBusObjectPointer->sendMsgBuf(TRANSMIT_ID, 0, 1, &randomNumber);

        if (sndStat == CAN_OK)
        {
            // -------------------------------
            // CHECK FOR ACK ERROR HERE               This doesnt really work with our transceiver, because this model doesnt need an ACK to send messages. 
            // -------------------------------
            byte errorAck = CANBusObjectPointer->getError();

            if (errorAck != 0)
            {
                Serial.print("Sent Number: ");
                Serial.print(randomNumber);
                Serial.print("  (ID: 0x");
                Serial.print(TRANSMIT_ID, HEX);
                Serial.println(")");

                Serial.print("⚠️ ERROR: No ACK on CAN bus!  Error Flags = 0x");
                Serial.println(errorAck, HEX);
            }
            else
            {
                Serial.print("Sent Number (ACK RECEIVED!): ");
                Serial.print(randomNumber);
                Serial.print("  (ID: 0x");
                Serial.print(TRANSMIT_ID, HEX);
                Serial.println(")");
            }
        }
        else
        {
            Serial.println("❌ Error sending CAN message...");
        }

        lastSendTime = millis();
    }
}

void CANCommunication::CANBusReceiveMsg(byte* receivedNumber)
{
    if (!digitalRead(CAN_INT))
    {
        CANBusObjectPointer->readMsgBuf(&rxId, &len, rxBuf);

        if (rxId == RECEIVE_ID && len == 1)
        {
            receivedNumber = &rxBuf[0];

            Serial.print("Received number: ");
            Serial.print(*receivedNumber);
            Serial.print("  (ID: 0x");
            Serial.print(rxId, HEX);
            Serial.println(")");

        }
    }
}