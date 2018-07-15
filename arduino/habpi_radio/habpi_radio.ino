#include <EEPROM.h>
#include <QueueList.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <XBee.h>
/**
 * HABPi Radio
 *
 * Running on a Arduino Pro Mini 5V, this program listens for messages from
 * the Raspberry Pi Zero, then passes them to the XBee 900HP radio for transmission.
 *
 * This code is written using a Finite State Machine, with three states:
 * Receiving, Sending and Emergency.
 *
 * The Arduino Pro Mini will listen for a message in the Receiving state, and once a message packet
 * is received, it will switch to the Sending state. If the message is a SENSOR_DATA type, it will also
 * save a copy into the EEPROM, overwriting any previously stored message. In the Sending state, the
 * Arduino Pro Mini will create an API Frame formatted message to send to the XBee 900HP radio, and
 * once that has been sent it will switch back to the Receiving state. In the event that the Arduino
 * Pro Mini does not receive a messsage from the Raspberry Pi Zero for more then EMERGENCY_TIMEOUT,
 * then it will switch to the Emergency state, where it will load the last saved SENSOR_DATA message
 * and broadcast this message through the XBee radio. If a message from the Raspberry Pi Zero is detected
 * the Arduino Pro Mini will switch back to the Receiving state.
 *
 * Author: Chris Capobianco
 * Date: 2017-06-18
 */

// Debug Flag
#define DEBUG 1

// EEPROM Memory Location
#define EEPROM_ADDRESS 0

// Address of Arduino Pro Mini
#define I2C_SLAVE_ADDRESS 0x04

// Maximum Packet Size
#define MAX_PACKET 100

#define rxFault 0x80
#define txFault 0x40
#define txRequest 0x20

struct {
  byte volatile command;
  byte volatile control; // rxFault:txFault:0:0:0:0:0:0
  float volatile latitude;
  float volatile longitude;
  float volatile altitude;
  float volatile speed;
  float volatile direction;
  float volatile heading;
  float volatile pitch;
  float volatile roll;
  float volatile temperature;
  float volatile pressure;
  float volatile height;
} commsTable;

byte volatile txTable[32];   // prepare data for sending over I2C

enum {
  SENSOR_DATA = 0,
  IMAGE_DATA = 1
};

// Define SoftwareSerial TX/RX pins
// Connect Arduino pin 10 to TX of XBee
uint8_t ssRX = 10;
// Connect Arduino pin 11 to RX of XBee
uint8_t ssTX = 11;
// Remember to connect all devices to a common Ground: XBee and Arduino
SoftwareSerial softSerial(ssRX, ssTX);

// Packet Format
typedef struct PACKET {
  char text[MAX_PACKET];
  uint8_t length;
  int msg_type, msg_index, msg_total;
} Packet;

// Message Format
typedef struct MESSAGE {
  int msg_type, msg_index, msg_total;
  float temp, baro, baro_alt;
  float magx, magy, magz;
  float mag_pitch, mag_roll, mag_heading;
  float gps_lat, gps_lon, gps_alt;
  float bat_volt;
  // Image chunk format?
} Message;

// Packet Queue
QueueList<Packet> PacketQueue;

// Message Queue
//QueueList<Message> MessageQueue;

// Packet Receive Buffer and Index
char ReceiveBuffer[MAX_PACKET] = "\0";
int ReceiveBufferIndex = 0;

// Create the XBee object
XBee xbee = XBee();

// XBee payload
uint8_t payload[MAX_PACKET] = { 0 };

// MAC (SH + SL) Address of Station Radio
#define STATION_SH 0x0013A200
#define STATION_SL 0x40F32EB0

// Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(STATION_SH, STATION_SL);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Counters
long interval = 10;
long counter = 1;
int msgType = 0;
long nerrors = 0;

// Flash LED pin n times with wait delays between each flash
void flashLed(int pin, int n, int wait) {
  for (int i = 0; i < n; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < n) {
      delay(wait);
    }
  }
}

// State flag
byte volatile state = 0;

// Send state function
void send() {
  if(DEBUG == 1) {
    Serial.println(F("Waiting to send data..."));
  }

  // If the packet queue isn't empty
  if (!PacketQueue.isEmpty()) {
    // Reset counter
    counter = 0;
    
    if(DEBUG == 1) {
      Serial.println(F("Found data to send!"));
    }

    // Pop a packet from the queue
    Packet packet = PacketQueue.pop();

    // If the message packet is a SENSOR_DATA type, keep a copy in EEPROM
    if(packet.msg_type == SENSOR_DATA) {
      EEPROM.write(EEPROM_ADDRESS, 0);
      EEPROM.put(EEPROM_ADDRESS, packet);
    }

    // Send packet to XBee radio for transmission
    Serial.print("Packet: ");
    Serial.println(packet.text);

    // Clear payload
    for(int i = 0; i < MAX_PACKET; i++) {
      payload[i] = 0;
    }

    // Set payload
    for(int i = 0; i < packet.length - 1; i++) {
      payload[i] = packet.text[i] - ' ';
    }

    // Send message
    xbee.send(zbTx);

    // flash TX indicator
    flashLed(LED_BUILTIN, 1, 100);

    // after sending a tx request, we expect a status response
    // wait up to half second for the status response
    if (xbee.readPacket(5000)) {
      // got a response, and should be a znet tx status              
      if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
        xbee.getResponse().getZBTxStatusResponse(txStatus);
        // get the delivery status, the fifth byte
        if (txStatus.getDeliveryStatus() == SUCCESS) {
          // success.  time to celebrate
          Serial.println("TX Success!");
          flashLed(LED_BUILTIN, 5, 50);
        } else {
          // the remote XBee did not receive our packet. is it powered on?
          Serial.println("TX Failure");
          flashLed(LED_BUILTIN, 3, 500);
        }
      }
    } else if (xbee.getResponse().isError()) {
      Serial.print("Error reading packet.  Error code: ");  
      Serial.println(xbee.getResponse().getErrorCode());
    } else {
      // local XBee did not provide a timely TX Status Response -- should not happen
      Serial.println("TX Response Timeout");
      flashLed(LED_BUILTIN, 2, 50);
    }
    
    //flashLed(LED_BUILTIN, 2, 50);
  } else {
    counter++;  
  }
}

// Emergencys state function
void emergency() {
  if(DEBUG == 1) {
    Serial.println(F("Emergency Broadcast"));
  }
  
  // Get backup SENSOR_DATA in EEPROM to broadcast
  Packet packet;
  EEPROM.get(EEPROM_ADDRESS, packet);
  Serial.print("Packet: ");
  Serial.println(packet.text);

  // Clear payload
  for(int i = 0; i < MAX_PACKET; i++) {
    payload[i] = 0;
  }

  // Set payload
  for(int i = 0; i < packet.length - 1; i++) {
    payload[i] = packet.text[i] - ' ';
  }

  // Send packet to XBee radio for transmission
  xbee.send(zbTx);

  // flash TX indicator
  flashLed(LED_BUILTIN, 1, 100);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(5000)) {
    // got a response, and should be a znet tx status              
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
        Serial.println("TX Success!");
        flashLed(LED_BUILTIN, 5, 50);
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        Serial.println("TX Failure");
        flashLed(LED_BUILTIN, 3, 500);
      }
    }
  } else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    Serial.println("TX Response Timeout");
    flashLed(LED_BUILTIN, 2, 50);
  }
  //flashLed(LED_BUILTIN, 3, 200);
}

// I2C master-write callback
void receiveEvent(int byteCount) {
  // Change state flag if we receive incoming data
  if(state == 1) {
    state = 0;
  }
  
  // While I2C data available, parse and store in buffer
  while (Wire.available()) {
    // Read next available byte
    unsigned char c = Wire.read();
    switch (c) {
      // Start byte received, reset buffer
      case 0x02: {
        memset(&ReceiveBuffer, 0, sizeof(ReceiveBuffer));
        ReceiveBuffer[0] = '\0';
        ReceiveBufferIndex = 0;
      } break;

      case 0x03: {
        // Stop byte received. Terminate buffer string, then copy to packet object and add to queue
        ReceiveBuffer[ReceiveBufferIndex++] = '\0';
        Packet p;
        strcpy(p.text, ReceiveBuffer);
        p.length = strlen(p.text);
        p.msg_type = msgType;
        msgType = 1 - msgType;
        PacketQueue.push(p);
      } break;

      default: {
        // Message byte received, increment index and store value
        ReceiveBuffer[ReceiveBufferIndex++] = c;
      } break;
    }
  }
}

// I2C slave-write callback
void requestEvent() {
  // Prepare internal buffer
  Wire.beginTransmission(I2C_SLAVE_ADDRESS);

  // Put data into the buffer
  Wire.write("OK", 2);

  // Send transmission
  byte result = Wire.endTransmission();

  if(result != 0) {
    Serial.println(F("Failed to send acknowledgment"));
  }
}

// Initialize I2C
void i2cInit() {
  // Set address of this device
  Wire.begin(I2C_SLAVE_ADDRESS);
  //TWBR = 12;

  // Set on receive callback function
  Wire.onReceive(receiveEvent);

  // Set on request callback function
  Wire.onRequest(requestEvent);
}

// Setup
void setup() {
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Set serial baud rate
  Serial.begin(9600);

  // Wait for serial port to connect. Needed for Leonardo and Due
  while(!Serial) {}

  // Initialize I2C
  i2cInit();

  // Initialize XBee
  softSerial.begin(9600);
  xbee.setSerial(softSerial);

  delay(5000);

  Serial.println(F("HABPI Radio Setup Complete"));
}

// Loop
void loop() {
  Serial.print(F("Counter: "));
  Serial.print(counter);
  Serial.print(F(", State: "));
  Serial.print(state);
  Serial.print(F(", # Errors: "));
  Serial.println(nerrors);
  if(counter > interval && state == 0) {
    // Change state flag and reset counter 
    counter = 0;
    state = 1;
    nerrors++;
    Serial.println(F("Raspberry Pi Not Responding"));
  }

  if(state == 0) {
    send();
  } else {
    emergency();
  }

  delay(5000);
}
