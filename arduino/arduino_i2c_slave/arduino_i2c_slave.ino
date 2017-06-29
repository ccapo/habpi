#include <QueueList.h>
#include <MemoryFree.h>
#include <Wire.h>

#define LED_RX 5
#define LED_TX 6
#define SLAVE_ADDRESS 0x04
#define MAX_PACKET 100

typedef struct PACKET {
  char Text[MAX_PACKET];
  uint8_t Length;
} Packet;

QueueList<Packet> PacketQueue;

char ReceiveBuffer[MAX_PACKET] = "\0";
int ReceiveBufferIndex = 0;

void i2cInit() {
  // I2C
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(onI2CReceiveData);
  Wire.onRequest(onI2CSendData);
}

void setup() {
  // initialize serial:
  Serial.begin(9600);
  //pinMode(LED_RX, OUTPUT);
  //pinMode(LED_TX, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  i2cInit();
  Serial.println("Setup Complete");
}

/**
 * Arduino loop
 */
void loop() {
  if (!PacketQueue.isEmpty()) {
    Packet packet = PacketQueue.pop();
    Serial.print("Packet: ");
    Serial.println(packet.Text);
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
  delay(1000);
}

/**
   Callback, triggered on I2C master-write operation.
*/
void onI2CReceiveData(int byteCount) {

  while (Wire.available()) {
    unsigned char c = Wire.read();

    switch (c) {
      // Start byte
      case 0x02: {
        memset(&ReceiveBuffer, 0, sizeof(ReceiveBuffer));
        ReceiveBuffer[0] = '\0';
        ReceiveBufferIndex = 0;
      } break;

      case 0x03: {
        // Stop byte
        ReceiveBuffer[ReceiveBufferIndex++] = '\0';
        Packet p;
        strcpy(p.Text, ReceiveBuffer);
        p.Length = strlen(p.Text);          
        PacketQueue.push(p);
      } break;

      default: {
        // Message byte
        ReceiveBuffer[ReceiveBufferIndex++] = c;
      } break;
    }
  }
}

/**
   Callback, triggered on I2C slave-write operation.
*/
void onI2CSendData() {
  Wire.write(0x02);
  Wire.write('O');
  Wire.write('K');
  Wire.write(0x03);
}

