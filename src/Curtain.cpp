#include "Curtain.h"

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

Curtain* Curtain::curtain_ = nullptr;

Curtain::Curtain() {
    //TODO: make offline version
    this->serverName = "http://192.168.0.174:8080/";
    this->BLEMAC = "0C:B8:15:CA:0B:92";

    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName + "get_device_by_mac");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{\"MAC\":\"" + this->BLEMAC + "\"}");
    
    // Send HTTP POST request
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    // Free resources
    http.end();


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