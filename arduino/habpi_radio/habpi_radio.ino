/**
 * HABPi Radio
 *
 * Running on a Arduino Pro Mini 5V, this program listens for messages from
 * the Raspberry Pi Zero, then passes them to the XBee 900HP radio for transmission.
 *
 * This code is written using a Finite State Machine, with two states:
 * NORMAL and EMERGENCY.
 *
 * The Arduino Pro Mini will listen for a messages on the SPI bus, and once a message is received,
 * it is validated against the provided checksum byte. If the checksum fails, the message is discarded.
 * If the message is a SENSOR type, a copy is saved into the EEPROM, overwriting any existing message.
 * The Arduino Pro Mini will then create an API Frame formatted message to send to the XBee 900HP radio.
 * In the event that the Arduino Pro Mini does not receive a messsage from the Raspberry Pi Zero after SPITIMEOUT,
 * then it will switch to the EMERGENCY state. Once in the EMERGENCY state, it will load the last saved SENSOR message
 * and broadcast this message through the XBee radio. If a message from the Raspberry Pi Zero is detected
 * the Arduino Pro Mini will switch back to the NORMAL state.
 * The Arduino Pro Mini reads the voltage sensors, and allows the Raspberry Pi Zero to request these values.
 *
 * This work is licensed under a Creative Commons 
 * Attribution-ShareAlike 4.0 International License.
 *
 * Author: Chris Capobianco
 * Date: 2017-06-18
 * Updated: 2018-08-07
 */
//#include <avr/power.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Tone.h>
#include <XBee.h>

// SPI Timeout (ms)
#define SPITIMEOUT        (300000)
volatile unsigned long spiTimeout = 0;

// Min. Broadcast delay (ms)
//#define BROADCASTDELAY    (5)
// Max. Broadcast delay with 16 attempts (ms)
#define BROADCASTDELAY    (80)
volatile unsigned long broadcastTime = 0;

// Battery measurement timeout (ms)
#define BATTERYDELAY      (30000)
volatile unsigned long batteryTime = 0;

// EEPROM memory location
#define EEPROMADDRESS     (0)

// Volatage sensors
#define RPIBATPIN         (A0)
#define ARDBATPIN         (A1)

// ASCII Codes in hexadecimal
#define NUL               (0x00)
#define STX               (0x02)
#define EOT               (0x04)
#define ENQ               (0x05)
#define ACK               (0x06)
#define NAK               (0x15)
#define US                (0x1F)

// SPI Commands
#define SENSOR            (0x60)
#define IMAGE             (0x70)
#define BATTERY           (0x90)
volatile uint8_t spiCommand = NUL;

// SPI States
#define IDLE              (0x00)
#define CMD               (0x01)
#define RECV              (0x02)
#define SEND              (0x03)
volatile uint8_t spiState = IDLE;

// Processing States
#define PENDING           (0x00)
#define READY             (0x1F)
volatile uint8_t dataReady = PENDING;
volatile uint8_t backupReady = PENDING;

// Message checksum and buffer index
volatile uint8_t i = 0;
volatile uint8_t chksum = 0;

// Message size (bytes)
#define BROADCASTSIZE     (100) // *Not* including checksum
#define IMAGESIZE         (100) // *Not* including checksum
#define SENSORSIZE        (72)  // *Not* including checksum
#define BATTERYSIZE       (9)   // Including checksum
uint8_t sensorData[SENSORSIZE] = {0};
uint8_t imageData[IMAGESIZE] = {0};
uint8_t batteryData[BATTERYSIZE] = {0};
//uint8_t batteryData[BATTERYSIZE] = {0xDB, 0x0F, 0x49, 0x40, 0xDB, 0x0F, 0xC9, 0x3F, 0x9A};

// Broadcast States
#define NORMAL            (0)
#define EMERGENCY         (1)
volatile uint8_t broadcastState = NORMAL;

// Define SoftwareSerial TX/RX pins
#define SS_RX             (4)
#define SS_TX             (5)

// Connect Arduino pin 4 to TX, and pin 5 to RX of XBee
// Remember to connect all devices to a common Ground: XBee and Arduino
SoftwareSerial softSerial(SS_RX, SS_TX);

// Create the XBee object
XBee xbee = XBee();

// XBee broadcast data
uint8_t broadcastData[BROADCASTSIZE] = {0};

// MAC (High + Low) Address of Station Radio
#define STATION_SH        (0x0013A200)
#define STATION_SL        (0x40F32EB0)

// XBee timeout (ms)
#define XBEETIMEOUT       (5000)

// Address of receiving XBee, request and response objects
XBeeAddress64 addr64 = XBeeAddress64(STATION_SH, STATION_SL);
ZBTxRequest zbTx = ZBTxRequest(addr64, broadcastData, BROADCASTSIZE);
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// XBee AT Commands
uint8_t atRE[2] = {'R','E'}; // Reset Settings
uint8_t atNI[2] = {'N','I'}; // Identifier
uint8_t argsNI[5] = {'H','A','B','P','I'}; // Identifier Args
uint8_t atHP[2] = {'H','P'}; // HP
uint8_t argsHP[1] = {'0'}; // HP Args
uint8_t atID[2] = {'I','D'}; // Network ID
uint8_t argsID[4] = {'2','0','1','8'}; // Network ID Args
uint8_t atCE[2] = {'C','E'}; // CE
uint8_t argsCE[1] = {'2'}; // CE Args
uint8_t atBD[2] = {'B','D'}; // BD
uint8_t argsBD[1] = {'3'}; // BD Args
uint8_t atAP[2] = {'A','P'}; // API Mode
uint8_t argsAP[1] = {'2'}; // API Mode Args
uint8_t atAO[2] = {'A','O'}; // AO
uint8_t argsAO[1] = {'0'}; // AO Args
uint8_t atDH[2] = {'D','H'}; // Station MAC Address (high)
uint8_t argsDH[4] = {0x00,0x13,0xA2,0x00}; // Station MAC Address (high) Args
uint8_t atDL[2] = {'D','L'}; // Station MAC Address (low)
uint8_t argsDL[4] = {0x40,0xF3,0x2E,0xB0}; // Station MAC Address (low) Args
uint8_t atWR[2] = {'W','R'}; // Write Settings

// XBee AT command Request and Response objects
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

// Tones
#define OCTAVE_OFFSET        (0)
#define isdigit(n)           (n >= '0' && n <= '9')
Tone tones;
int notes[] = {0,
NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7};
char melody[] = "ff6_victory:d=4,o=5,b=140:32d6,32p,32d6,32p,32d6,32p,d6,a#,c6,16d6,8p,16c6,2d6,a,g,a,16g,16p,c6,16c6,16p,b,16c6,16p,b,16b,16p,a,g,f#,16g,16p,1e,a,g,a,16g,16p,c6,16c6,16p,b,16c6,16p,b,16b,16p,a,g,a,16c6,16p,1d6";

// Sensor message
struct sensor_msg_t {
  // Message Type and Proximity Flag
  uint8_t type, proximityFlag;
  
  // GPS Data
  uint8_t gps_nsats, gps_status, gps_mode;
  float gps_lat, gps_lon, gps_alt, gps_gspd, gps_dir, gps_vspd;
  
  // MPL3115A2 Data
  float mpl_temp, mpl_pres, mpl_alt;

  // AHRS: NXP_FXOS8700_FXAS21002C Data
  float ahrs_head, ahrs_pitch, ahrs_roll;

  // DHT11 Data
  float dht_temp, dht_relh;
  
  // Battery Data
  float bat_rpi, bat_ard;
} sensorMsg;

// Battery message
struct battery_msg_t {
  // Message Type
  uint8_t type;
  
  // Battery Data
  float bat_rpi, bat_ard;
} batteryMsg;

// Setup function
void setup(void) {
  //clock_prescale_set( clock_div_2 );

  //Serial.begin(115200);
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);

  // Have to send on *Master In, Slave Out*
  pinMode(MISO, OUTPUT);

  // Set pins for voltage sensor
  pinMode(RPIBATPIN, INPUT);
  pinMode(ARDBATPIN, INPUT);

  // Read battery voltages
  readBatteryVoltages();

  // Turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // Turn off interrupts
  SPCR &= ~_BV(SPIE);

  // Initialize SPI register
  SPDR = NUL;

  // Setup tone library
  tones.begin(LED_BUILTIN);

  // Register XBee
  //softSerial.begin(9600);
  //xbee.setSerial(softSerial);

  // Wait for XBee to respond
  //delay(XBEETIMEOUT);

  // Prepare and send RE command
  prepareAtCommand(atRE, NULL);
  
  // Prepare and send NI command
  prepareAtCommand(atNI, argsNI);

  // Prepare and send HP command
  prepareAtCommand(atHP, argsHP);

  // Prepare and send ID command
  prepareAtCommand(atID, argsID);

  // Prepare and send CE command
  prepareAtCommand(atCE, argsCE);

  // Prepare and send BD command
  prepareAtCommand(atBD, argsBD);

  // Prepare and send AP command
  prepareAtCommand(atAP, argsAP);

  // Prepare and send AO command
  prepareAtCommand(atAO, argsAO);

  // Prepare and send DH command
  prepareAtCommand(atDH, argsDH);

  // Prepare and send DL command
  prepareAtCommand(atDL, argsDL);

  // Prepare and send WR command
  prepareAtCommand(atWR, NULL);

  // Set battery message type
  batteryMsg.type = BATTERY;

  //Serial.println(F("Ready"));
}

// Loop function
void loop(void) {
  // Read battery voltages after BATTERYDELAY
  if (abs(millis() - batteryTime) >= static_cast<unsigned long>(BATTERYDELAY)) {
    batteryTime = millis();
    readBatteryVoltages();
  }

  // Execute appropriate broadcast state
  if(broadcastState == NORMAL) {
    normalState();
  } else {
    emergencyState();
  }
}

void normalState() {
  //Serial.println(F("Normal State"));

  // If data is ready to be processed
  if (dataReady == READY) {
    // Wait for XBee to broadcast message
    if (abs(millis() - broadcastTime) >= static_cast<unsigned long>(BROADCASTDELAY)) {
      // Send packet to XBee radio for transmission
      //xbee.send(zbTx);

      // After sending a tx request, we expect a status response
      // Wait up to XBEETIMEOUT milliseconds for the status response
      //if (xbee.readPacket(XBEETIMEOUT)) {
        // Got a response, and should be a znet tx status              
        //if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
          //xbee.getResponse().getZBTxStatusResponse(txStatus);
          // Get the delivery status
          //if (txStatus.isSuccess()) {
            // Success
            //Serial.println(F("TX Success!"));
            //flashLed(LED_BUILTIN, 5, 50);
          //} else {
            // The remote XBee did not receive our packet, is it powered on?
            //Serial.println(F("TX Failure"));
            //flashLed(LED_BUILTIN, 3, 500);
          //}
        //}
      //} else if (xbee.getResponse().isError()) {
        //Serial.print(F("Error reading packet. Error code: "));  
        //Serial.println(xbee.getResponse().getErrorCode());
      //} else {
        // Local XBee did not provide a timely TX Status Response -- should not happen
        //Serial.println(F("TX Response Timeout"));
        //flashLed(LED_BUILTIN, 2, 50);
      //}

      // Clear broadcast data
      for(uint8_t j = 0; j < BROADCASTSIZE; j++) {
        broadcastData[j] = 0;
      }

      // Play success melody if proximity flag is set
      if (sensorMsg.proximityFlag == US) {
        play_rtttl(melody);
      }

      broadcastTime = millis();
      dataReady = PENDING;
    }
  } else {
    // Wait until the SPI transmission has completed
    if ((SPSR & (1 << SPIF)) != 0) {
      spiHandler();
      spiTimeout = millis();
    }

    if (abs(millis() - spiTimeout) > static_cast<unsigned long>(SPITIMEOUT)) {
      spiTimeout = millis();
      i = 0;
      chksum = 0;
      spiState = IDLE;
      spiCommand = NUL;
      SPDR = NUL;
      broadcastState = EMERGENCY;
      //Serial.println("Emergency State");
    }
  }
}

void emergencyState() {
  //Serial.println(F("Emergency State"));

  if (backupReady == READY) {
    // Wait for XBee to broadcast message
    if (abs(millis() - broadcastTime) >= static_cast<unsigned long>(BROADCASTDELAY)) {
      // Read sensor data from EEPROM
      EEPROM.get(EEPROMADDRESS, sensorData);

      // Populate broadcast data
      for(uint8_t j = 0; j < SENSORSIZE; j++) {
        broadcastData[j] = sensorData[j];
      }

      // Send packet to XBee radio for transmission
      //xbee.send(zbTx);

      // After sending a tx request, we expect a status response
      // Wait up to XBEETIMEOUT milliseconds for the status response
      //if (xbee.readPacket(XBEETIMEOUT)) {
        // Got a response, and should be a znet tx status              
        //if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
          //xbee.getResponse().getZBTxStatusResponse(txStatus);
          // Get the delivery status
          //if (txStatus.isSuccess()) {
            // Success
            //Serial.println(F("TX Success!"));
            //flashLed(LED_BUILTIN, 5, 50);
          //} else {
            // The remote XBee did not receive our packet, is it powered on?
            //Serial.println(F("TX Failure"));
            //flashLed(LED_BUILTIN, 3, 500);
          //}
        //}
      //} else if (xbee.getResponse().isError()) {
        //Serial.print(F("Error reading packet. Error code: "));  
        //Serial.println(xbee.getResponse().getErrorCode());
      //} else {
        // Local XBee did not provide a timely TX Status Response -- should not happen
        //Serial.println(F("TX Response Timeout"));
        //flashLed(LED_BUILTIN, 2, 50);
      //}
      
      // Clear broadcast data
      for(uint8_t j = 0; j < BROADCASTSIZE; j++) {
        broadcastData[j] = 0;
      }

      // Play SOS if proximity flag is set
      if (sensorMsg.proximityFlag == US) {
        play_sos();
      }

      broadcastTime = millis();
      backupReady = PENDING;
    }
  } else {
    // Check if a SPI transmission has completed
    if ((SPSR & (1 << SPIF)) != 0) {
      spiHandler();
      spiTimeout = millis();
      broadcastState = NORMAL;
      //Serial.println("Normal State");
    }

    // Read battery voltages after BATTERYDELAY
    if (abs(millis() - batteryTime) >= static_cast<unsigned long>(BATTERYDELAY)) {
      batteryTime = millis();
      backupReady = READY;
    }
  }
}

// SPI handler
void spiHandler() {
  static bool first = true;
  uint8_t spi = SPDR;

  //Serial.print(F("spiState = 0x")); Serial.println(spiState, HEX);
  //Serial.print(F("spiCommand = 0x")); Serial.println(spiCommand, HEX);

  // State machine
  switch (spiState) {
    case IDLE: {
      // If Master is starting a new session, change to 
      // CMD state and ACK message
      if (spi == STX) {
        spiState = CMD;
        SPDR = ACK;
      } else {
        // Otherwise reset, and send back NUL
        i = 0;
        chksum = 0;
        spiState = IDLE;
        spiCommand = NUL;
        SPDR = NUL;
      }
    } break;
    case CMD: {
      // Save sent command, determine if command is for
      // transmitting or requesting data, and ACK message
      spiCommand = spi;
      SPDR = ACK;
      if (spiCommand < 0x80) {
        spiState = RECV;
      } else {
        spiState = SEND;
      }
    } break;
    case RECV: {
      // Handle incoming data
      switch (spiCommand) {
        case SENSOR: {
          // If this is the first time here, skip this byte
          if (first == true) {
            first = false;
            SPDR = ACK;
          } else {            
            // Store sent data
            if (i < SENSORSIZE) {
              sensorData[i++] = spi;
              chksum += spi;
              SPDR = ACK;
            } else {
              // Store sent checksum value
              uint8_t chksumSent = spi;

              // Compute checksum and compare with sent value
              chksum = 0xFF - (chksum & 0xFF);

              // Reset first flag
              first = true;

              if (chksum == chksumSent) {
                i = 0;
                chksum = 0;
                spiCommand = NUL;
                spiState = IDLE;
                dataReady = READY;
                SPDR = ACK;

                deserialize(sensorData, &sensorMsg);

                // Store copy in EEPROM
                EEPROM.put(EEPROMADDRESS, sensorData);

                // Populate broadcast data
                for(uint8_t j = 0; j < SENSORSIZE; j++) {
                  broadcastData[j] = sensorData[j];
                }
                //Serial.println(F("ACK Sensor Data"));
              } else {
                i = 0;
                chksum = 0;
                spiCommand = NUL;
                spiState = IDLE;
                dataReady = PENDING;
                SPDR = NAK;
                //Serial.print(F("chksum = 0x")); Serial.println(chksum, HEX);
                //Serial.print(F("chksumSent = 0x")); Serial.println(chksumSent, HEX);
                //Serial.println(F("NAK Sensor Data"));
              }
            }
          }
        } break;
        case IMAGE: {
          // If this is the first time here, skip this byte
          if (first == true) {
            first = false;
            SPDR = ACK;
          } else {
            // Store sent data
            if (i < IMAGESIZE) {
              imageData[i++] = spi;
              chksum += spi;
              SPDR = ACK;
            } else {
              // Store sent checksum value
              uint8_t chksumSent = spi;

              // Compute checksum and compare with sent value
              chksum = 0xFF - (chksum & 0xFF);

              // Reset first flag
              first = true;

              if (chksum == chksumSent) {
                // If checksum values match, send ACK messasge
                i = 0;
                chksum = 0;
                spiCommand = NUL;
                spiState = IDLE;
                dataReady = READY;
                SPDR = ACK;

                // Populate broadcast data
                for(uint8_t j = 0; j < IMAGESIZE; j++) {
                  broadcastData[j] = imageData[j];
                }
                //Serial.println(F("ACK Image Data"));
              } else {
                // Otherwise send NAK message
                i = 0;
                chksum = 0;
                spiCommand = NUL;
                spiState = IDLE;
                dataReady = PENDING;
                SPDR = NAK;
                //Serial.print(F("chksum = 0x")); Serial.println(chksum, HEX);
                //Serial.print(F("chksumSent = 0x")); Serial.println(chksumSent, HEX);
                //Serial.println(F("NAK Image Data"));
              }
            }
          }
        } break;
        default: {
          // Unrecognized command
          i = 0;
          chksum = 0;
          spiState = IDLE;
          spiCommand = NUL;
          SPDR = NUL;
        } break;
      }
    } break;
    case SEND: {
      // Handle outgoing data
      switch (spiCommand) {
        case BATTERY: {
          if (i < BATTERYSIZE) {
            SPDR = batteryData[i++];
          } else {
            i = 0;
            chksum = 0;
            spiCommand = NUL;
            spiState = IDLE;
            SPDR = ACK;
          }
        } break;
        default: {
          // Unrecognized command
          i = 0;
          chksum = 0;
          spiState = IDLE;
          spiCommand = NUL;
          SPDR = NUL;
        } break;
      }
    } break;
    default: {
      // Unrecogized state, reset and send back NUL
      i = 0;
      chksum = 0;
      spiState = IDLE;
      spiCommand = NUL;
      SPDR = NUL;
    } break;
  }
}

// XBee API Packet Checksum
uint8_t checksum(uint8_t len, uint8_t *buffer) {
  // See http://knowledge.digi.com/articles/Knowledge_Base_Article/Calculating-the-Checksum-of-an-API-Packet
  uint8_t check = 0;

  for (uint8_t i = 0; i < len; i++) {
    check += buffer[i];
  }

  return 0xFF - (check & 0xFF);
}

// Prepare AT command to XBee
void prepareAtCommand(uint8_t *command, uint8_t *arguments) {
  if (arguments != NULL) {
    uint8_t len = sizeof(arguments);

    atRequest.setCommand(command);
    atRequest.setCommandValue(arguments);
    atRequest.setCommandValueLength(len);
    //sendAtCommand();
  } else {
    atRequest.setCommand(command);
    atRequest.clearCommandValue();
    //sendAtCommand();
  }
}

// Send AT command to XBee
void sendAtCommand() {
  //Serial.println("Sending command to the XBee");

  // send the command
  xbee.send(atRequest);

  // Wait up to 5 seconds for the status response
  if (xbee.readPacket(XBEETIMEOUT)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        //Serial.print("Command [");
        //Serial.print(atResponse.getCommand()[0]);
        //Serial.print(atResponse.getCommand()[1]);
        //Serial.println("] was successful!");

        if (atResponse.getValueLength() > 0) {
          //Serial.print("Command value length is ");
          //Serial.println(atResponse.getValueLength(), DEC);

          //Serial.print("Command value: ");
          //for (int i = 0; i < atResponse.getValueLength(); i++) {
          //  Serial.print(atResponse.getValue()[i], HEX);
          //  Serial.print(" ");
          //}
          //Serial.println("");
        }
      } else {
        //Serial.print("Command return error code: ");
        //Serial.println(atResponse.getStatus(), HEX);
      }
    } else {
      //Serial.print("Expected AT response but got ");
      //Serial.print(xbee.getResponse().getApiId(), HEX);
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      //Serial.print("Error reading packet.  Error code: ");  
      //Serial.println(xbee.getResponse().getErrorCode());
    } else {
      //Serial.println("No response from radio");  
    }
  }
}

// Read battery voltages, storing values in batteryData
void readBatteryVoltages() {
  int voltageReading;
  float voltage;  
  float *p = (float *)batteryData;

  // Read voltage of Raspberry Pi batteries, and store in batteryData
  //voltageReading = analogRead(RPIBATPIN);
  voltageReading = 271;
  voltage = static_cast<float>(voltageReading)/40.92;
  *p = voltage;
  p++;

  // Read voltage of Arduino batteries, and store in batteryData
  //voltageReading = analogRead(ARDBATPIN);
  voltageReading = 297;
  voltage = static_cast<float>(voltageReading)/40.92;
  *p = voltage;
  p++;

  // Compute and store checksum
  uint8_t chk = checksum(BATTERYSIZE - 1, batteryData);
  uint8_t *q = (uint8_t *)p;
  *q = chk;
  q++;
}

// Deserialize sensor message
void deserialize(uint8_t *data, sensor_msg_t *msg) {
  // Message Type
  uint8_t *p = (uint8_t *)data;
  msg->type = *p;
  p++;

  // Proximity flag
  msg->proximityFlag = *p;
  p++;

  // GPS
  msg->gps_nsats = *p;
  p++;
  msg->gps_status = *p;
  p++;
  msg->gps_mode = *p;
  p++;

  float *q = (float *)p;
  msg->gps_lat = *q;
  q++;
  msg->gps_lon = *q;
  q++;
  msg->gps_alt = *q;
  q++;
  msg->gps_gspd = *q;
  q++;
  msg->gps_dir = *q;
  q++;
  msg->gps_vspd = *q;
  q++;

  // AHRS
  msg->ahrs_head = *q;
  q++;
  msg->ahrs_pitch = *q;
  q++;
  msg->ahrs_roll = *q;
  q++;

  // MPL
  msg->mpl_temp = *q;
  q++;
  msg->mpl_pres = *q;
  q++;
  msg->mpl_alt = *q;
  q++;

  // DHT + VBAT
  msg->dht_temp = *q;
  q++;
  msg->dht_relh = *q;
  q++;
  msg->bat_rpi = *q;
  q++;
  msg->bat_ard = *q;
  q++;
}

void play_rtttl(char *p) {
  // Absolutely no error checking in here
  uint8_t default_dur = 4;
  uint8_t default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  uint8_t note;
  uint8_t scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd') {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  //Serial.print("ddur: "); Serial.println(default_dur, 10);

  // get default octave
  if(*p == 'o') {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  //Serial.print("doct: "); Serial.println(default_oct, 10);

  // get BPM
  if(*p == 'b') {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  //Serial.print("bpm: "); Serial.println(bpm, 10);

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  //Serial.print("wn: "); Serial.println(wholenote, 10);

  // now begin note loop
  while(*p) {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch(*p) {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#') {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.') {
      duration += duration/2;
      p++;
    }
  
    // now, get scale
    if(isdigit(*p)) {
      scale = *p - '0';
      p++;
    } else {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',') {
      p++;       // skip comma for next note (or we may be at the end)
    }

    // now play the note

    if(note) {
      //Serial.print("Playing: ");
      //Serial.print(scale, 10); Serial.print(' ');
      //Serial.print(note, 10); Serial.print(" (");
      //Serial.print(notes[(scale - 4) * 12 + note], 10);
      //Serial.print(") ");
      //Serial.println(duration, 10);
      tones.play(notes[(scale - 4) * 12 + note]);
      delay(duration);
      tones.stop();
    } else {
      //Serial.print("Pausing: ");
      //Serial.println(duration, 10);
      delay(duration);
    }
  }
}

// Morse Code Functions
void dot() {
  freqout(2048, 50);
  delay(50);
}

void dash() {
  freqout(2048, 150);
  delay(50);
}

// Frequency output
void freqout(int freq, int t) { 
  int hperiod;    
  long cycles, i; 

  hperiod = 500000 / (freq - 7);          // subtract 7 us to make up for digitalWrite overhead - determined empirically                
  cycles = ((long)freq * (long)t) / 1000;   
  
  digitalWrite(LED_BUILTIN, HIGH);        // turn LED ON
  for (i = 0; i <= cycles; i++) {         // square wave generation
    digitalWrite(LED_BUILTIN, HIGH);  
    delayMicroseconds(hperiod); 
    digitalWrite(LED_BUILTIN, LOW);  
    delayMicroseconds(hperiod - 1);
  } 
  digitalWrite(LED_BUILTIN, LOW);         // turn LED OFF                
}

// SOS in Morse Code
void play_sos() {                      
  dot(); dot(); dot(); 
  delay(100); // pause between characters is 50. Between words is 150, but we already have 50 from the last character
  dash(); dash(); dash();
  delay(100); 
  dot(); dot(); dot();
}
