#include "Curtain.h"

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

Curtain* Curtain::curtain_ = nullptr;

Curtain::Curtain() {
    //TODO: make offline version
    //TMP
    this->BLEMAC = "0C:B8:15:CA:0B:92";

    const int stepsPerRevolution = 2048;
    this->stepper = new Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

    this->serverName = "http://192.168.0.174:3000/";
    String endpoint = "get_device_by_mac";

    HTTPClient http;
    http.addHeader("Content-Type", "application/json");
 
    http.begin(this->serverName + endpoint); //Specify the URL and certificate
    int httpCode = http.POST("\"MAC\": \""+ this->BLEMAC +"\"");                                                  //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);

        JSONVar json = JSON.parse(payload);
        if (JSON.typeof(json) == "undefined") {
            Serial.println("Parsing input failed!");
            this->stepper->setSpeed(26);
            this->YPosClosed = 6000;
        } else {
            //tmp
            Serial.print("Device: ");
            Serial.println(json["device"]["name"]);

            int speed = (int)json["device"]["motorSpeed"];
            
            Serial.print("Speed set to: ");
            Serial.println(speed);
            this->stepper->setSpeed(speed);
            this->YPosClosed = (int)json["device"]["YPosClosed"];
        }
      } else {
        Serial.println(httpCode);
        Serial.println("Error on HTTP request");
    }
 
    http.end(); 

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

void Curtain::stepperPowerOff() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

int Curtain::makeResponselessAPICall(String endpoint, String payload) {
    HTTPClient http;
    http.addHeader("Content-Type", "application/json");
 
    http.begin(this->serverName + endpoint);
    int httpCode = http.POST(payload);

    if (httpCode < 0) {
        Serial.println("Error on HTTP request");
    }
    return httpCode;
}

void Curtain::setOwnerCredentials(const std::string& s) {
    this->ownerEmail = "";
    this->ownerPassword = "";
    size_t separator = s.find(" \t ");
    Serial.println(separator);
    for(int i = 1; i < separator; ++i) {
        // Serial.println(s[i]);
        this->ownerEmail += s[i];
    }
    Serial.println("");
    for(int i = separator + 3; i < s.length(); ++i) {
        this->ownerPassword += s[i];
    }
    // Serial.println("");
    Serial.println("Done reading");
    Serial.print("email: ");
    Serial.println(this->ownerEmail);
    Serial.print("password: ");
    Serial.println(this->ownerPassword);
}