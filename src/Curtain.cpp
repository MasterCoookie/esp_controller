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

    JSONVar data;
    data["MAC"] = this->BLEMAC;

    HTTPClient http;
    http.begin(this->serverName + endpoint);
    http.addHeader("Content-Type", "application/json");
 
    Serial.println(JSON.stringify(data));
    int httpCode = http.POST(JSON.stringify(data));                                                  //Make the request
 
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
            this->deviceID = json["device"]["_id"];

            //TMPdev
            this->ownerEmail = "jan.kocurek@proton.me";
            this->ownerPassword = "dupa1234";
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

int Curtain::makeResponselessAPICall(const String& endpoint, JSONVar& doc) {
    HTTPClient http;
    
    Serial.println("Making http request to "+ this->serverName + endpoint);
    Serial.println("With data: "+ JSON.stringify(doc));

    http.begin(this->serverName + endpoint);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(JSON.stringify(doc));

    if (httpCode < 0) {
        Serial.println("Error on HTTP request");
    }
    return httpCode;
}

JSONVar Curtain::makeJSONResposiveAPICall(const String& endpoint, JSONVar& doc) {
    HTTPClient http;
    JSONVar empty;


    Serial.println("Making http request to "+ this->serverName + endpoint);
    Serial.println("With data: "+ JSON.stringify(doc));

    http.begin(this->serverName + endpoint);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(JSON.stringify(doc));
    if (httpCode < 0) {
        Serial.println("Error on HTTP request: ");
        Serial.println(httpCode);
        return empty;
    } else if(httpCode == 200) {
        String payload = http.getString();
        //TMP
        Serial.println(httpCode);
        Serial.println(payload);
        JSONVar json = JSON.parse(payload);
        if (JSON.typeof(json) == "undefined") {
            Serial.println("Parsing input failed!");
            return empty;
        } else {
            return json;
        }
    } else {
        Serial.println("Code: " + httpCode);
        return empty;
    }
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

void Curtain::appendUserAuth(JSONVar& doc) {
    doc["email"] = this->ownerEmail;
    doc["password"] = this->ownerPassword;
    doc["deviceID"] = this->deviceID;
    //tmp
    // Serial.println(JSON.stringify(doc));
}

void Curtain::checkPendingEvent() {
    // Serial.println(this->pendingEvent.keys().length()); 
    if(this->pendingEvent.keys().length() <= 0) {
        JSONVar payload;
        payload["getTimeAsTimestamp"] = true;
        payload["getDummyData"] = true;
        this->appendUserAuth(payload);
        this->pendingEvent = this->makeJSONResposiveAPICall("check_pending_event", payload);
        Serial.println("event:" + JSON.stringify(this->pendingEvent));
    } else {
        this->epochTime = getTime();
        // Serial.print("Epoch Time: ");
        int diff = ((int)this->pendingEvent["event"]["eventTime"]) - this->epochTime;
        // Serial.print("Time o event: ");
        // Serial.println(diff);
        if(diff < 0) {
            //do event
            Serial.print("Going to Y: ");
            Serial.println(this->pendingEvent["event"]["targetYpos"]);
            int y_step = (int)this->pendingEvent["event"]["targetYpos"] - this->currentYPos;
            this->setCurrentYPos(y_step);
            this->stepperStep(y_step);
        }
    }
}

unsigned long Curtain::getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}