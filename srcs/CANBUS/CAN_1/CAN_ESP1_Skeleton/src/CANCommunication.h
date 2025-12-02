#pragma once


#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS_PIN SS
#define CAN_INT 4
#define ESP_MISO_PIN MISO
#define ESP_MOSI_PIN MOSI
#define ESP_CLK_PIN SCK


class CANCommunication
{
    public:
    /*class constructor. TRANSMIT_ID is own CAN address. RECEIVE_ID is the address where it needs to send to. 
    also initializes MCP_CAN object*/
    CANCommunication(unsigned long TRANSMIT_ID, unsigned long RECEIVE_ID);

    /*Initialize SPI and CAN bus*/
    void CANBusInit();

    /*Send a random number via CAN bus every 5 seconds*/
    void CANBusSendMsg();

    /*Receive message from CAN bus and print it out. Message is stored in receivedNumber*/
    void CANBusReceiveMsg(byte* receivedNumber);

    private:
    MCP_CAN* CANBusObjectPointer;
    unsigned long lastSendTime;
    long unsigned int rxId;
    unsigned char len;
    unsigned char rxBuf[8];
    unsigned long TRANSMIT_ID; // test ID from Ian: 0x200
    unsigned long RECEIVE_ID; // test ID from Ian: 0x300
};
