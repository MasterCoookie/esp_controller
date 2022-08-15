#include "SetupCallback.h"

SetupCallback::SetupCallback(Curtain* curtain) {
    this->curtain = curtain;
}

void SetupCallback::onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 1) {
      this->curtain->setConfigMode(true);
      if(value == "U" && curtain->getRotorState() == RotorState::STOP) {
        this->curtain->setRotorState(RotorState::UP);
      } else if(value == "S" && curtain->getRotorState() != RotorState::STOP) {
        this->curtain->setRotorState(RotorState::STOP);
      } else if(value == "D" && curtain->getRotorState() == RotorState::STOP) {
        this->curtain->setRotorState(RotorState::DOWN);
      } else if(value == "O") {
        //set current pos as upper limit
        this->curtain->setYPosClosed(this->curtain->getYPosClosed() - this->curtain->getCurrentYPos());
        this->curtain->resetCurrentYPos();
      } else if(value == "C") {
        this->curtain->setYPosClosed(this->curtain->getCurrentYPos());
      }
    } else {
      //handling speed change request
      int speed = std::stoi(value);
      Serial.print("Chnaging speed to");
      Serial.println(speed);
      this->curtain->setStepperSpeed(speed);
    }
  }