#include "Curtain.h"

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

Curtain* Curtain::curtain_ = nullptr;

Curtain::Curtain() {
    //TMP
    this->YPosClosed = 6000;

    const int stepsPerRevolution = 2048;
    this->stepper = new Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

    this->rotorState = RotorState::STOP;
    this->currentYPos = 0;
    this->configMode = false;
}

Curtain* Curtain::getInstance() {
    if(curtain_ == nullptr) {
        curtain_ = new Curtain();
    }
    return curtain_;
}