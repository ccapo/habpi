// Test code for Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code shows how to listen to the GPS module in an interrupt
// which allows the program to have more 'freedom' - just parse
// when a new NMEA sentence is available! Then access data when
// desired.
//
// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746
// Pick one up today at the Adafruit electronics shop 
// and help support open source hardware & software! -ada

#include <math.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include "Mahony.h"
#include "Madgwick.h"

/* Assign a unique ID to this sensor at the same time */
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);
Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);

// accelerometer offsets
float acc_offsets[3]            = { 0.239354F, -0.465763F, 9.978260F };

// Offsets applied to raw x/y/z mag values
float mag_offsets[3]            = { -77.62F, -125.41F, 59.46F };

// Soft iron error compensation matrix
float mag_softiron_matrix[3][3] = { {  1.010,  0.028,  0.014 },
                                    {  0.028,  1.017,  0.008 },
                                    {  0.015,  0.008,  0.976 } };

float mag_field_strength        = 50.11F;

// Offsets applied to compensate for gyro zero-drift error for x/y/z
// Raw values converted to rad/s based on 250dps sensitiviy (1 lsb = 0.00875 rad/s)
float rawToDPS = 0.00875F;
float dpsToRad = 0.017453293F;
//float gyro_zero_offsets[3]      = { 0.0F, 0.0F, 0.0F };
float gyro_zero_offsets[3]      = { 500.0F * rawToDPS * dpsToRad,
                                    500.0F * rawToDPS * dpsToRad,
                                    315.0F * rawToDPS * dpsToRad };

/*float gyro_zero_offsets[3]      = { 175.0F * rawToDPS * dpsToRad,
                                   -729.0F * rawToDPS * dpsToRad,
                                    101.0F * rawToDPS * dpsToRad };*/

// Mahony is lighter weight as a filter and should be used
// on slower systems
//Mahony filter;
Madgwick filter;

// If using software serial, keep this line enabled
// (you can change the pin numbers to match your wiring):
SoftwareSerial mySerial(3, 2);

Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  false

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = true;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

sensors_event_t aevent, mevent, gevent;

void setup()  
{
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Serial.println("Adafruit GPS library basic test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  Serial.println("NXP PFXOS8700 FXAS21002 Test");

  // Initialize the sensors.
  if(!gyro.begin())
  {
    /* There was a problem detecting the gyro ... check your connections */
    Serial.println("Ooops, no gyro detected ... Check your wiring!");
    while(1);
  }

  /* Initialise the sensor */
  if(!accelmag.begin(ACCEL_RANGE_2G))
  {
    /* There was a problem detecting the FXOS8700 ... check your connections */
    Serial.println("Ooops, no FXOS8700 detected ... Check your wiring!");
    while(1);
  }

  // Filter expects 25 samples per second
  filter.begin(70);

  delay(1000);

  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();
void loop() {
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    
    if (GPS.fix && GPS.fixquality) {
      Serial.print("Fix: "); Serial.print((int)GPS.fix);
      Serial.print(" quality: "); Serial.print((int)GPS.fixquality); 
      Serial.print(" satellites: "); Serial.println((int)GPS.satellites);
      Serial.print("Location: ");
      Serial.print(GPS.latitudeDegrees, 6);
      Serial.print(", "); 
      Serial.print(GPS.longitudeDegrees, 6);
      Serial.print(", ");
      Serial.println(GPS.altitude, 2);
      
      Serial.print("Speed (knots): "); Serial.print(GPS.speed);
      Serial.print(", Angle: "); Serial.println(GPS.angle);

      /* Get a new sensor event */
      gyro.getEvent(&gevent);
      accelmag.getEvent(&aevent, &mevent);

      // Apply mag offset compensation (base values in uTesla)
      float x = mevent.magnetic.x - mag_offsets[0];
      float y = mevent.magnetic.y - mag_offsets[1];
      float z = mevent.magnetic.z - mag_offsets[2];

      // Apply mag soft iron error compensation
      float mx = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];
      float my = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
      float mz = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];

      // Apply gyro zero-rate error compensation
      float gx = gevent.gyro.x - gyro_zero_offsets[0];
      float gy = gevent.gyro.y - gyro_zero_offsets[1];
      float gz = gevent.gyro.z - gyro_zero_offsets[2];

      // The filter library expects gyro data in degrees/s, but adafruit sensor
      // uses rad/s so we need to convert them first (or adapt the filter lib
      // where they are being converted)
      gx *= 180.0F/M_PI;
      gy *= 180.0F/M_PI;
      gz *= 180.0F/M_PI;

      float a[3] = {aevent.acceleration.x, aevent.acceleration.y, aevent.acceleration.z};

      // Update the filter
      filter.update(gx, gy, gz,
                  a[0], a[1], a[2],
                  mx, my, mz);

      // Print the orientation filter output in quaternions.
      // This avoids the gimbal lock problem with Euler angles when you get
      // close to 180 degrees (causing the model to rotate or flip, etc.)
      /*float qw, qx, qy, qz;
      filter.getQuaternion(&qw, &qx, &qy, &qz);
      Serial.print("Quat: ");
      Serial.print(qw);
      Serial.print(" ");
      Serial.print(qx);
      Serial.print(" ");
      Serial.print(qy);
      Serial.print(" ");
      Serial.println(qz);*/

      // Print the orientation filter output
      // Note: To avoid gimbal lock you should read quaternions not Euler
      // angles, but Euler angles are used here since they are easier to
      // understand looking at the raw values. See the ble fusion sketch for
      // and example of working with quaternion data.
      float roll = filter.getRoll();
      float pitch = filter.getPitch();
      float heading = filter.getYaw();
      Serial.print("Orientation: ");
      Serial.print(heading);
      Serial.print(", ");
      Serial.print(pitch);
      Serial.print(", ");
      Serial.println(roll);
    } else {
      Serial.print("Fix: "); Serial.print((int)GPS.fix);
      Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    }
  }

  delay(100);
}
