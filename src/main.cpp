#include <Arduino.h>
#include <Stepper.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

const int stepsPerRevolution = 2048;
enum class RotorState { UP, STOP, DOWN };
RotorState rotorState = RotorState::STOP;

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

#define SERVICE_UUID        "eda3620e-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_UUID "f67783e2-0e6a-11ed-861d-0242ac120002"

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() == 1) {
        if(value == "U" && rotorState == RotorState::STOP) {
          rotorState = RotorState::UP;
        } else if(value == "S" && rotorState != RotorState::STOP) {
          rotorState = RotorState::STOP;
        } else if(value == "D" && rotorState == RotorState::STOP) {
          rotorState = RotorState::DOWN;
        }
      } else if (value.length() > 1) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup() {
  myStepper.setSpeed(17);
  // initialize the serial port
  Serial.begin(115200);
}

void loop() {

}