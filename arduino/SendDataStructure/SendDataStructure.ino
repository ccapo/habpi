/**
 * Copyright (c) 2015 Daniel Noyes. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <XBee.h>

/*
 This example is for Series 1 (10C8 or later firmware) or Series 2 XBee  radios.
 Two XBee are needed for this example.
 This example will be using two xbee attached to an arduino (Leonardo/Mega) for two serial bus.
 One XBee will be transmitting a data packet while another XBee will receive and output the data to the serial bus.
 */

//Configuration for the Example:
#define COORDINATOR_SH 0x0013A200
#define COORDINATOR_SL 0x40BF1B18

#define STATE 1 /* State: 0: transmit
                          1: receieve */

//Example Data Structure to be used in this example.
typedef struct XBeeStruct {
  int val1;
  float val2;
} XBeeDataStruct;

//Create the Data Structure
static XBeeDataStruct XBeeData;

//Setup XBee
XBee xbee = XBee();

// 64-bit addressing: This is the SH + SL address of remote XBee
XBeeAddress64 addr64 = XBeeAddress64(COORDINATOR_SH, COORDINATOR_SL);
// unless you have MY on the receiving radio set to FFFF, this will be received as a RX16 packet
Tx64Request tx = Tx64Request(addr64, (uint8_t *)&XBeeData, sizeof(XBeeDataStruct));

TxStatusResponse txStatus = TxStatusResponse();

//Response Handler
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

//Receieve Variables
uint8_t* data = 0;
int len = 0;

void setup() {
  // initialize usb serial(leonardo) communications at 9600 bps
  Serial.begin(9600);
  // initialize external serial(leonardo) communications at 9600 bps
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
  delay(5000); //Wait for Xbee to fully initalize
}


void ReadData() {
  //The Example will grab the analog value off of Pin A5 and save it as both int and float.
  float pin5f = analogRead(5);
  int pin5i = pin5f;

  //update data
  XBeeData.val1 = pin5i;
  XBeeData.val2 = pin5f;
}

void PrintData() {
  Serial.print("Data Obtained:");
  Serial.print("\tVal1 = ");
  Serial.print(XBeeData.val1);
  Serial.print("\tVal2 = ");
  Serial.print(XBeeData.val2);
  Serial.println();
}

void SendData() {
  xbee.getNextFrameId();
  Serial.println("Sending data over XBee");
  xbee.send(tx);
  Serial.println("Checking for Status Response");

  if (xbee.readPacket(5000)) {
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      Serial.println("Received Response");
      if ( txStatus.getStatus() == SUCCESS ) {
        Serial.println("Successful Sent");
      } 
      else {
        Serial.println("Failure Send, Check Remote Unit");
      }
    } 
    else {
      Serial.println("[Error]: No Response");
    }
  } 
  else {
    if (xbee.getResponse().isError()) {
      Serial.print("[Error]: Reading Packet: ");
      Serial.println(xbee.getResponse().getErrorCode());
    } 
    else {
      // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected.
      Serial.println("[Error]: No Response Status Provided, Reconfigure/Reset XBee");
    }
  }
}

void ReceiveData() {
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
      //Serial.println("Received Response");

      Serial.print("Packet Info:");
      Serial.print("\tType = ");
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
        xbee.getResponse().getRx16Response(rx16);
        data = rx16.getData();
        len = rx16.getDataLength();
        Serial.print("RX16");
      } 
      else {
        xbee.getResponse().getRx64Response(rx64);
        data = rx64.getData();
        len = rx64.getDataLength();
        Serial.print("RX64");
      }

      Serial.print("\tSize = ");
      Serial.println(len);
      XBeeData = (XBeeDataStruct &)*data;
      //send the new data out to test
      //SendData();
    } 
    else {
      Serial.println("[Error]: Not The Correct Response");
    }
  } 
  else {
    if (xbee.getResponse().isError()) {
      Serial.print("[Error]: Reading Packet: ");
      Serial.println(xbee.getResponse().getErrorCode());
    } 
    else {
      // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected.
      Serial.println("[Error]: No Response Status Provided, Reconfigure/Reset XBee");
    }
  }
}

void loop() {
  if(STATE == 0) { //transmit
    ReadData();
    PrintData();
    SendData();
  } 
  else {
    ReceiveData();
    PrintData();
  }
  delay(5000);
}

