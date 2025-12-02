#ifndef CANCOMMUNICATIONS_H

#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS 5
#define CAN_INT 4
#define TRANSMIT_ID 0x200
#define RECEIVE_ID 0x300

MCP_CAN CAN0(5);
unsigned long lastSendTime = 0;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

void CANBusInit();

void CANBusSendMsg();
void CANBusReceiveMsg(byte* receivedNumber);

#endif
