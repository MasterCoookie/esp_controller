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

    //makes API call, returns only HTTP status code
    int makeResponselessAPICall(const String& endpoint, JSONVar& payload);
    //makes API call, returns JSON response
    JSONVar makeJSONResposiveAPICall(const String& endpoint, JSONVar& payload);
    //append user credentials and device id to JSON passed by reference
    void appendUserAuth(JSONVar& doc);

    //decodes string which contains user auth, then writes it to EEPROM
    void setOwnerCredentials(const std::string& s);

    //Singleton getter
    static Curtain* getInstance();

    //make API call that checks if there is an event to happen soon. Also, if event is already queued on device, this method will run it when the time comes
    void checkPendingEvent();

    //writes given data to EEPROM starting form given address
    void EEPROMWrite(const char* data, unsigned short int startingAddrr);
    //reads data from EEPROM starting form given address
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