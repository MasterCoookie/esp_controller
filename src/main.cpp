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
#define CHARACTERISTIC_2_UUID "2250634e-8aa7-4e3e-b3a1-31d4bbe40127"

class RemoteCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() == 1) {
      if(value == "U" && rotorState == RotorState::STOP) {
        rotorState = RotorState::UP;
        Serial.println("Going UP");
      } else if(value == "S" && rotorState != RotorState::STOP) {
        rotorState = RotorState::STOP;
        Serial.println("STOPPING");
      } else if(value == "D" && rotorState == RotorState::STOP) {
        rotorState = RotorState::DOWN;
        Serial.println("Going DOWN");
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

class SetupCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    int speed = std::stoi(value);
    Serial.println("Speed change reqested:");
    Serial.print(speed);
    Serial.println(" rpm");
    myStepper.setSpeed(speed);
  }
};



void setup() {
  myStepper.setSpeed(25);
  // initialize the serial port
  Serial.begin(115200);

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(CHARACTERISTIC_2_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new RemoteCallback());
  pCharacteristic2->setCallbacks(new SetupCallback());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("Setup complete");
}

void loop() {
  if(rotorState == RotorState::STOP) {
    delay(100);
  } else if(rotorState == RotorState::UP){
    myStepper.step(150);
  } else if(rotorState == RotorState::DOWN){
    myStepper.step(-150);
  }
}