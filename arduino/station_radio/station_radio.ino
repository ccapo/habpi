/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
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

/**
 * This example is for Series 2 XBee
 * Receives a ZB RX packet and prints the packet to softserial
 */

XBee xbee = XBee();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

// Define SoftwareSerial TX/RX pins
// Connect Arduino pin 10 to TX of XBee
uint8_t ssRX = 10;
// Connect Arduino pin 11 to RX of XBee
uint8_t ssTX = 11;
// Remember to connect all devices to a common Ground: XBee and Arduino
SoftwareSerial softSerial(ssRX, ssTX);

//Example Data Structure to be used in this example.
typedef struct XBeeStruct {
  int val1;
  float val2;
} XBeeDataStruct;

//Create the Data Structure
static XBeeDataStruct XBeeData;

//Receieve Variables
uint8_t *data = 0;
int len = 0;

int statusLed = 13;
int errorLed = 13;
int dataLed = 13;

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
  pinMode(dataLed,  OUTPUT);
   
  // start serial
  Serial.begin(9600);

  softSerial.begin(9600);
  xbee.setSerial(softSerial);

  delay(5000);
  Serial.println("Starting up!");
}

// continuously reads packets, looking for ZB Receive or Modem Status
void loop() {
    
  //xbee.readPacket();

  // Read packet, waiting 5 seconds
  if (xbee.readPacket(5000)) {

    // Got something
    Serial.println("Got Packet");
         
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // got a zb rx packet
      
      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);
    
      Serial.println("Got an rx packet!");

      Serial.print("RX Option: ");
      Serial.println(rx.getOption());
      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        // the sender got an ACK
        Serial.println("packet acknowledged");
      } else {
        Serial.println("packet not acknowledged");
      }
      
      Serial.print("checksum is ");
      Serial.println(rx.getChecksum(), HEX);

      Serial.print("packet length is ");
      Serial.println(rx.getPacketLength(), DEC);

      // Store received data
      data = rx.getData();

      // Transform data to XBeeDataStruct object
      XBeeData = (XBeeDataStruct &)*data;

      Serial.print("Data Obtained:");
      Serial.print("\tVal1 = ");
      Serial.print(XBeeData.val1);
      Serial.print("\tVal2 = ");
      Serial.print(XBeeData.val2);
      Serial.println();
      
      // Print raw data
      /*for (int i = 0; i < rx.getDataLength(); i++) {
        Serial.print("payload [");
        Serial.print(i, DEC);
        Serial.print("] is ");
        Serial.println(rx.getData()[i], HEX);
      }*/
      
      // Print raw data + frame
     /*for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
       Serial.print("frame data [");
       Serial.print(i, DEC);
       Serial.print("] is ");
       Serial.println(xbee.getResponse().getFrameData()[i], HEX);
     }*/
    }
  } else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
      xbee.getResponse().getModemStatusResponse(msr);
      // the local XBee sends this response on certain events, like association/dissociation
      
      if (msr.getStatus() == ASSOCIATED) {
        // yay this is great.  flash led
        Serial.println("Associated!");
        flashLed(statusLed, 10, 10);
      } else if (msr.getStatus() == DISASSOCIATED) {
        // this is awful.. flash led to show our discontent
        Serial.println("Dissassociated");
        flashLed(errorLed, 10, 10);
      } else {
        // another status
        Serial.print("Other modem status: ");
        Serial.println(msr.getStatus());
        flashLed(statusLed, 5, 10);
      }
  } else if (xbee.getResponse().isError()) {
    Serial.print("Error Code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  }

  delay(5000);
}
