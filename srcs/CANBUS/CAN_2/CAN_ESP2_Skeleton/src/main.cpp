#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS 5
#define CAN_INT 4

MCP_CAN CAN0(5);

#define TRANSMIT_ID 0x300
#define RECEIVE_ID 0x200

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

unsigned long lastSendTime = 0;

void setup()
{
  Serial.begin(115200);
  SPI.begin(18,19,23, CAN_CS);

  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
  {
    Serial.println("MCP2515 Initialized Successfully!");
  }
  else
  {
    Serial.println("Error Initializing MCP2515...");
  }

  CAN0.setMode(MCP_NORMAL);

  Serial.println("Node Ready!");
}

void loop()
{
    //--------------------------------------
    // SEND A RANDOM NUMBER EVERY 5 SECONDS
    //--------------------------------------

    if (millis() - lastSendTime >= 5000)
    {
        byte randomNumber = random(1,41);
        byte sndStat = CAN0.sendMsgBuf(TRANSMIT_ID, 0, 1, &randomNumber);

        if (sndStat == CAN_OK)
        {
            // -------------------------------
            // CHECK FOR ACK ERROR HERE
            // -------------------------------
            byte errorAck = CAN0.getError();

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


    //--------------------------------------
    //          RECEIVE MESSAGES
    //--------------------------------------

    if (!digitalRead(CAN_INT))
    {
        CAN0.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId == RECEIVE_ID && len == 1)
        {
            byte receivedNumber = rxBuf[0];

            Serial.print("Received number: ");
            Serial.print(receivedNumber);
            Serial.print("  (ID: 0x");
            Serial.print(rxId, HEX);
            Serial.println(")");

        }
    }
}





