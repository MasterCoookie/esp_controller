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
        //set current pos as closed
        this->curtain->setYPosClosed(this->curtain->getCurrentYPos());
        
        JSONVar payload;
        payload["YPosClosed"] = this->curtain->getYPosClosed();
        this->curtain->appendUserAuth(payload);
        const int response = this->curtain->makeResponselessAPICall("update_device/", payload);
        if(response == 200) {
          Serial.println("YPosClosed changed in API");
        } else {
          Serial.println(response);
        }
      }
    } else if (value.length() > 1) {
      Serial.println(value[0]);
      if(value[0] == 'S') {
        //handling speed change request
        int speed = std::stoi(value.substr(1, value.length()));
        Serial.print("Chnaging speed to ");
        Serial.println(speed);
        this->curtain->setStepperSpeed(speed);
  
        JSONVar payload;
        payload["motorSpeed"] = speed;
        this->curtain->appendUserAuth(payload);

        Serial.println(this->curtain->makeResponselessAPICall("update_device/", payload));
      } else if(value[0] == 'C') {
        // save user credentials
        //TODO check for faulty declarations
        if(value.find(" \t ") != std::string::npos) {
          this->curtain->setOwnerCredentials(value);
        } else {
          Serial.println("no separator found");
        }
        
      }
    }
  }