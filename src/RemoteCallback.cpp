#include "RemoteCallback.h"

RemoteCallback::RemoteCallback(Curtain* curtain) {
    this->curtain = curtain;
}

void RemoteCallback::onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 1) {
      curtain->setConfigMode(false);
      if(value == "U" && curtain->getRotorState() == RotorState::STOP) {
        curtain->setRotorState(RotorState::UP);
        Serial.println("Going UP");
      } else if(value == "S" && curtain->getRotorState() != RotorState::STOP) {
        curtain->setRotorState(RotorState::STOP);
        Serial.println("STOPPING");
      } else if(value == "D" && curtain->getRotorState() == RotorState::STOP) {
        curtain->setRotorState(RotorState::DOWN);
        Serial.println("Going DOWN");
      } else if(value == "O") {
        curtain->setRotorState(RotorState::OPEN);
        Serial.println("Opening");
      } else if(value == "C") {
        curtain->setRotorState(RotorState::CLOSE);
        Serial.println("Closing");
      }
    } else if (value.length() > 1) {
      Serial.println("*********");
      Serial.print("New value: ");

      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
      }
        
      Serial.println();
      Serial.println("*********");
    }
  }