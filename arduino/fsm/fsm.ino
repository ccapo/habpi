#include <FiniteStateMachine.h>
#include <Entropy.h>

// Number of states in the FSM
const byte NUMBER_OF_STATES = 3;

// State callback functions
void receive() {
  digitalWrite(LED_BUILTIN, LOW);
}

void send() {
  digitalWrite(LED_BUILTIN, HIGH);

  // Pause
  delay(500);
}

void emergency() {
  // Three dots
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  delay(100);

  // Three dashes
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);

  delay(100);

  // Three dots
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

// Initialize states
State receiveState = State(receive);
State sendState = State(send);
State emergencyState = State(emergency);

// Initialize state machine, start in state: receiveState
FSM fsm = FSM(receiveState);

// Counter variable
uint8_t random_byte = 0;
 
void setup() {
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Wait for serial port to connect. Needed for Leonardo and Due
  Serial.begin(9600);
  while(!Serial) {
    ;
  }

  // This routine sets up the watch dog timer with interrupt handler to maintain a
  // pool of real entropy for use in sketches.  This mechanism is relatively slow
  // since it will only produce a little less than two 32-bit random values per 
  // second.
  Entropy.initialize();
}

void loop(){
  // Pick a number between 1 and 3
  random_byte = Entropy.random(1, 4);

  if(random_byte == 1 && !fsm.isInState(sendState)) {
    fsm.transitionTo(sendState);
    Serial.println("Send State");
  } else if(random_byte == 2 && !fsm.isInState(receiveState)) {
    fsm.transitionTo(receiveState);
    Serial.println("Receive State");
  } else if(random_byte == 3 && !fsm.isInState(emergencyState)) {
    fsm.transitionTo(emergencyState);
    Serial.println("Emergency State");
  } else {
    if(fsm.isInState(sendState)) {
      Serial.println("Send State");
    } else if(fsm.isInState(receiveState)) {
      Serial.println("Receive State");
    } else {
      Serial.println("Emergency State");
    }
  }

  // Update the state machine
  fsm.update();
  delay(1000);
}
