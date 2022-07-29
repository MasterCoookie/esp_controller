#include <Arduino.h>
#include <Stepper.h>

const int stepsPerRevolution = 2048;

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup() {
  // set the speed at 5 rpm
  myStepper.setSpeed(10);
  // initialize the serial port
  Serial.begin(115200);
}

void loop() {
 Serial.println("clockwise");
  myStepper.step(stepsPerRevolution);
  delay(1000);

  // step one revolution in the other direction:
  Serial.println("counterclockwise");
  myStepper.step(-stepsPerRevolution);
  delay(1000);
}