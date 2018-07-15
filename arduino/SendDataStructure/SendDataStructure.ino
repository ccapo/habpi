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
#include <SoftwareSerial.h>

/*
 This example is for Series 1 (10C8 or later firmware) or Series 2 XBee  radios.
 Two XBee are needed for this example.
 This example will be using two xbee attached to an arduino (Leonardo/Mega) for two serial bus.
 One XBee will be transmitting a data packet while another XBee will receive and output the data to the serial bus.
 */

//Configuration for the Example:
#define COORDINATOR_SH 0x0013A200
#define COORDINATOR_SL 0x40F32EB0

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
ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t *)&XBeeData, sizeof(XBeeDataStruct));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Define NewSoftSerial TX/RX pins
// Connect Arduino pin 8 to TX of usb-serial device
uint8_t ssRX = 10;
// Connect Arduino pin 9 to RX of usb-serial device
uint8_t ssTX = 11;

// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial softSerial(ssRX, ssTX);

//Receieve Variables
uint8_t *data = 0;
int len = 0;
int counter = 0;

int statusLed = 13;
int errorLed = 13;

void flashLed(int pin, int times, int wait) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}

void setup() {
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  
  // initialize usb serial(leonardo) communications at 9600 bps
  Serial.begin(9600);

  // Initialize XBee
  softSerial.begin(9600);
  xbee.setSerial(softSerial);
  delay(5000); //Wait for Xbee to fully initalize
  Serial.println(F("Ready to Send"));  
}


void ReadData() {
  //The Example will grab the analog value off of Pin A5 and save it as both int and float.
  float pin5f = M_PI; //analogRead(5);
  int pin5i = pin5f;

  //update data
  XBeeData.val1 = ++counter;
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
  xbee.send(zbTx);
  Serial.println("Checking for Status Response");

  if (xbee.readPacket(5000)) {
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      Serial.println("Received Response");
      if ( txStatus.getDeliveryStatus() == SUCCESS ) {
        Serial.println("Successfully Sent");
        flashLed(statusLed, 5, 50);
      } 
      else {
        Serial.println("Failure Send, Check Remote Unit");
        flashLed(errorLed, 3, 500);
      }
    } 
    else {
      Serial.println("[Error]: No Response");
      flashLed(errorLed, 5, 500);
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
  ReadData();
  PrintData();
  SendData();
  delay(5000);
}
