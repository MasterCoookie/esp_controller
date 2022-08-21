#pragma once
#include <Stepper.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "time.h"
#include "EEPROM.h"

#define EEPROM_SIZE 32
#define EEPROM_CHUNKS 4

enum class RotorState { UP, STOP, DOWN, OPEN, CLOSE };

class Curtain {
public:
    //TODO: make offline version
    void initializeOnline();
    void initializeOffline();

    const bool isInOnlineMode() const { return this->onlineMode; }

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

    int makeResponselessAPICall(const String& endpoint, JSONVar& payload);
    JSONVar makeJSONResposiveAPICall(const String& endpoint, JSONVar& payload);
    void appendUserAuth(JSONVar& doc);

    void setOwnerCredentials(const std::string& s);

    static Curtain* getInstance();

    void checkPendingEvent();

    void EEPROMWrite(const char* data, unsigned short int startingAddrr);
    String EEPROMRead(unsigned short int startingAddr);
private:
    Curtain();
    void initFromJSON(JSONVar& json);
    unsigned long getTime();
    unsigned long epochTime;

    bool onlineMode;

    static Curtain* curtain_;

    String ownerEmail;
    String ownerPassword;

    String serverName;
    String BLEMAC;
    String deviceID;

    RotorState rotorState;

    JSONVar pendingEvent;

    int YPosClosed;
    int currentYPos;
    bool configMode;

    Stepper* stepper;
};