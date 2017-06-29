/*
   Arduino Sketch to show data being sent between a Raspberry Pi
   and Arduino using SMBus and Wire.h
   Raspberry Pi is Master
   We have chosen the slave address for the Arduino to be the first
   'safe' unreserved address, viz 8.

   Arduino "reads" a temperature and a light sensor and allows
   the Raspberry Pi to interrogate the Arduino.
   Arduino has an RGB led attached on pwm pins so that
   brightness and color can be varied under control of the Raspberry Pi.

   This work is licensed under a Creative Commons 
   Attribution-ShareAlike 4.0 International License.

   Software by Mike Ochtman
   Find me on LinkedIn and drop me a note if you found this useful!
*/
#include <Wire.h>

#define rxFault 0x80
#define txFault 0x40
#define txRequest 0x20
#define slaveAddress 4

struct {
  byte volatile command;
  byte volatile control; // rxFault:txFault:0:0:0:0:0:0
  float volatile temperature;
  float volatile light;
  byte volatile brightR;
  byte volatile brightG;
  byte volatile brightB;
} commsTable;

byte volatile txTable[32];   // prepare data for sending over I2C
bool volatile dataReady = false; // flag to trigger a Serial printout after an I2C event
// use volatile for variables that will be used in interrupt service routines.
// "Volatile" instructs the compiler to get a fresh copy of the data rather than try to
// optimise temporary registers before using, as interrupts can change the value.

void setup() {

  Serial.begin(9600);  // For monitoring on the PC
  while (!Serial) {};  // Wait for the Serial port to initialise properly
  Serial.println("I2C Wire Library Tests\n");

  Wire.begin(slaveAddress);  // I2C slave address 8 setup.
  Wire.onReceive(i2cReceive);  // register our handler function with the Wire library
  Wire.onRequest(i2cTransmit);  // register data return handler

  commsTable.temperature = 23.95; // simulate a temperature
  commsTable.light = 6.97;  // simulate a light level
  printCommsTable();
  printTxTable();

}

void loop() {

  if (dataReady) {
    printCommsTable();
    dataReady = false;
  }

  if (commsTable.control > 0) {
    printTxTable();
    commsTable.control = 0;
  }

}

//====================================================
/*
   i2cReceive:
   Parameters: integer, number of bytes in rx buffer
   Returns: none
   Function called by twi interrupt service when master sends
   information to the slave, or when master sets up a
   specific read request.
   Incoming data must be processed according to the
   Interface Specification decided upon.
   The first byte sent is a command byte, and this informs
   the slave how to react to the transmission.
   See the end of this document for the Interface Specification
   for this example. Typically, the MSB of "command" is used to
   signal 'read' or 'write' instructions.
*/
void i2cReceive(int byteCount) {
  // if byteCount is zero, the master only checked for presence
  // of the slave device, triggering this interrupt. No response necessary
  if (byteCount == 0) return;

  // our Interface Specification says commands in range 0x000-0x7F are
  // writes TO this slave, and expects nothing in return.
  // commands in range 0x80-0xFF are reads, requesting data FROM this device
  byte command = Wire.read();
  commsTable.command = command;
  if (command < 0x80) {
    i2cHandleRx(command);
  } else {
    i2cHandleTx(command);
  }
  dataReady = true;
}

/*
   i2cTransmit:
   Parameters: none
   Returns: none
   Next function is called by twi interrupt service when twi detects
   that the Master wants to get data back from the Slave.
   Refer to Interface Specification for details of what data must be sent.
   A transmit buffer (txTable) is populated with the data before sending.
*/
void i2cTransmit() {
  // byte *txIndex = (byte*)&txTable[0];
  byte numBytes = 0;
  int t = 0; // temporary variable used in switch occasionally below

  // check whether this request has a pending command.
  // if not, it was a read_byte() instruction so we should
  // return only the slave address. That is command 0.
  if ((commsTable.control & txRequest) == 0) {
    // this request did not come with a command, it is read_byte()
    commsTable.command = 0; // clear previous command
  }
  // clear the rxRequest bit; reset it for the next request
  commsTable.control &= ~txRequest;

  // If an invalid command is sent, we write nothing back. Master must
  // react to the crickets.
  switch (commsTable.command) {
    case 0x00: // send slaveAddress.
      txTable[0] = slaveAddress;
      numBytes = 1;
      break;
    case 0x81:  // send temperature
      t = int(round(commsTable.temperature * 100));
      txTable[1] = (byte)(t >> 8);
      txTable[0] = (byte)(t & 0xFF);
      numBytes = 2;
      break;
    case 0x82:  // send light
      t = int(round(commsTable.light * 100));
      txTable[1] = (byte)(t >> 8);
      txTable[0] = (byte)(t & 0xFF);
      numBytes = 2;
      break;
    case 0x90: // send RGB
      txTable[0] = commsTable.brightR;
      txTable[1] = commsTable.brightG;
      txTable[2] = commsTable.brightB;
      numBytes = 3;
      break;
    case 0x91: // send RGB Red channel
      txTable[0] = commsTable.brightR;
      numBytes = 1;
      break;
    case 0x92: // send RGB Green channel
      txTable[0] = commsTable.brightG;
      numBytes = 1;
      break;
    case 0x93: // send RGB Blue channel
      txTable[0] = commsTable.brightB;
      numBytes = 1;
      break;
    default:
      // If an invalid command is sent, we write nothing back. Master must
      // react to the sound of crickets.
      commsTable.control |= txFault;
  }
  if (numBytes > 0) {
    Wire.write((byte *)&txTable, numBytes);
  }
}

/*
   i2cHandleRx:
   Parameters: byte, the first byte sent by the I2C master.
   returns: byte, number of bytes read, or 0xFF if error
   If the MSB of 'command' is 0, then master is sending only.
   Handle the data reception in this function.
*/
byte i2cHandleRx(byte command) {
  // If you are here, the I2C Master has sent data
  // using one of the SMBus write commands.
  byte result = 0;
  // returns the number of bytes read, or FF if unrecognised
  // command or mismatch between data expected and received

  switch (command) {
    case 0x0A:  // read three bytes in a block to set brightness and color
      if (Wire.available() == 3) { // good write from Master
        commsTable.brightR = Wire.read();
        commsTable.brightG = Wire.read();
        commsTable.brightB = Wire.read();
        result = 3;
      } else {
        result = 0xFF;
      }
      break;

    case 0x0B:
      if (Wire.available() == 1) { // good write from Master
        commsTable.brightR = Wire.read();
        result = 1;
      } else {
        result = 0xFF;
      }
      break;

    case 0x0C:
      if (Wire.available() == 1) { // good write from Master
        commsTable.brightG = Wire.read();
        result = 1;
      } else {
        result = 0xFF;
      }
      break;

    case 0x0D:
      if (Wire.available() == 1) { // good write from Master
        commsTable.brightB = Wire.read();
        result = 1;
      } else {
        result = 0xFF;
      }
      break;

    default:
      result = 0xFF;
  }

  if (result == 0xFF) commsTable.control |= rxFault;
  return result;

}

/*
   i2cHandleTx:
   Parameters: byte, the first byte sent by master
   Returns: number of bytes received, or 0xFF if error
   Used to handle SMBus process calls
*/
byte i2cHandleTx(byte command) {
  // If you are here, the I2C Master has requested information

  // If there is anything we need to do before the interrupt
  // for the read takes place, this is where to do it.
  // Examples are handling process calls. Process calls do not work
  // correctly in SMBus implementation of python on linux,
  // but it may work on better implementations.

  // signal to i2cTransmit() that a pending command is ready
  commsTable.control |= txRequest;
  return 0;

}

void printCommsTable() {
  String builder = "";
  builder = "commsTable contents:";
  Serial.println(builder);
  builder = "  command: ";
  builder += String(commsTable.command, HEX);
  Serial.println(builder);
  builder = "  control: ";
  builder += String(commsTable.control, HEX);
  Serial.println(builder);
  builder = "  temperature: ";
  builder += commsTable.temperature;
  builder += (char)186;  // the "º" symbol
  builder += "C";
  Serial.println(builder);
  builder = "  light level: ";
  builder += commsTable.light;
  builder += " lux";
  Serial.println(builder);
  builder = "  brightness Red: ";
  builder += commsTable.brightR;
  Serial.println(builder);
  builder = "  brightness Green: ";
  builder += commsTable.brightG;
  Serial.println(builder);
  builder = "  brightness Blue: ";
  builder += commsTable.brightB;
  Serial.println(builder);
  Serial.println();
}

void printTxTable() {
  Serial.println("Transmit Table");
  for (byte i = 0; i < 32; i++) {
    Serial.print("  ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(txTable[i]);
  }
  Serial.println();
}
/* Interface Specification
    Data in a table thus:
      byte purpose
      0: command
      1: control
     2-5: Current Temperature (read-only)
     6-9: Current light level (read only)
     10: Brightness for RED read/write
     11: Brightness for GREEN read/write
     12: Brightness for BLUE read/write

     Commands:
     Write with no command: Ignore
     Read with no command: Return slave address
     Command 0x81: read temperature. Integer returned, int(round(temp*100))
     Command 0x82: read light level, Integer returned, int(round(lux*100))
     Command 0x0A: Write three bytes to RGB
     Command 0x0B: Write single byte brightness red;
     Command 0x0C: Write single byte brightness green;
     Command 0x0D: Write single byte brightness blue;
     Command 0x90: read three bytes brightness RGB
     Command 0x91: read single byte brightness red;
     Command 0x92: read single byte brightness green;
     Command 0x93: read single byte brightness blue;

     All other values are ignored, no data returned.

*/
