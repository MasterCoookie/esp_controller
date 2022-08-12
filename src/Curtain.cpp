#include "Curtain.h"

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

Curtain* Curtain::curtain_ = nullptr;

Curtain::Curtain() {
    //TODO: make offline version
    this->BLEMAC = "0C:B8:15:CA:0B:92";

    const char* server_name = "http://192.168.0.174:8080/";

    WiFiClientSecure client;
    // if (WiFi.waitForConnectResult() = WL_CONNECTED) {
    //     Serial.println("Not connected");
    // } else {

    

        client.setInsecure();//skip verification
        if (!client.connect(server_name, 443))
            Serial.println("Connection failed!");
        else {
            Serial.println("Connected to server!");
            client.println("POST https://esp.requestcatcher.com/test HTTP/1.0");
            client.println("Dupa: zmitac");


            while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                Serial.println("headers received");
                break;
            }
            }
            // if there are incoming bytes available
            // from the server, read them and print them:
            while (client.available()) {
            char c = client.read();
            Serial.write(c);
            }

            client.stop();
        }
    // }


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