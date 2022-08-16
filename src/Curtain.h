#pragma once
#include <Stepper.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

enum class RotorState { UP, STOP, DOWN, OPEN, CLOSE };

class Curtain {
public:
    const RotorState getRotorState() const { return this->rotorState; }
    void setRotorState(const RotorState state) { this->rotorState = state; }
    void setStepperSpeed(const int speed) { this->stepper->setSpeed(speed); }

    void stepperStep(const int& step) { this->stepper->step(step); }
    void stepperPowerOff();

    const bool getConfigMode() const { return this->configMode; }
    void setConfigMode(const bool mode) { this->configMode = mode; }

    const int getCurrentYPos() const { return this->currentYPos; }
    void resetCurrentYPos() { this->currentYPos = 0; }
    void setCurrentYPos(const int& y) { this->currentYPos = y; }
    void incrementYPos(const int& y) { this->currentYPos += y; }

    const int getYPosClosed() const { return this->YPosClosed; }
    void setYPosClosed(const int& y) { this->YPosClosed = y; }

    int makeResponselessAPICall(const char* endpoint, const char* payload);

    static Curtain* getInstance();

private:
    Curtain();
    static Curtain* curtain_;

    String serverName;
    String BLEMAC;

    RotorState rotorState;

    int YPosClosed;
    int currentYPos;
    bool configMode;

    Stepper* stepper;
};