#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

#include "SetupCallback.h"

//TMP
const char* ssid = "Maszt 5G test 300% mocy";
const char* password = "aqq123321qqa";

Curtain* curtain = new Curtain();


AsyncWebServer server(80);

#define SERVICE_UUID          "eda3620e-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_UUID   "f67783e2-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_2_UUID "2250634e-8aa7-4e3e-b3a1-31d4bbe40127"

#define STEP 150


class RemoteCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 1) {
      curtain->setConfigMode(false);
      if(value == "U" && curtain->getRotorState() == RotorState::STOP) {
        curtain->setRotorState(RotorState::UP);
        WebSerial.println("Going UP");
        Serial.println("Going UP");
      } else if(value == "S" && curtain->getRotorState() != RotorState::STOP) {
        curtain->setRotorState(RotorState::STOP);
        Serial.println("STOPPING");
        WebSerial.println("STOPPING");
      } else if(value == "D" && curtain->getRotorState() == RotorState::STOP) {
        curtain->setRotorState(RotorState::DOWN);
        Serial.println("Going DOWN");
        WebSerial.println("Going DOWN");
      } else if(value == "O") {
        curtain->setRotorState(RotorState::OPEN);
        Serial.println("Opening");
        WebSerial.println("Opening");
      } else if(value == "C") {
        curtain->setRotorState(RotorState::CLOSE);
        Serial.println("Closing");
        WebSerial.println("Closing");
      }
    } else if (value.length() > 1) {
      Serial.println("*********");
      Serial.print("New value: ");
      WebSerial.println("*********");
      WebSerial.print("New value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
        WebSerial.print(value[i]);
      }
        
      Serial.println();
      Serial.println("*********");
      WebSerial.println();
      WebSerial.println("*********");
    }
  }
};



// class SetupCallback: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharacteristic) {
//     std::string value = pCharacteristic->getValue();
//     if (value.length() == 1) {
//       curtain->setConfigMode(true);
//       if(value == "U" && curtain->getRotorState() == RotorState::STOP) {
//         curtain->setRotorState(RotorState::UP);
//       } else if(value == "S" && curtain->getRotorState() != RotorState::STOP) {
//         curtain->setRotorState(RotorState::STOP);
//       } else if(value == "D" && curtain->getRotorState() == RotorState::STOP) {
//         curtain->setRotorState(RotorState::DOWN);
//       } else if(value == "O") {
//         //set current pos as upper limit
//         curtain->resetCurrentYPos();
//       } else if(value == "C") {
//         //set current pos as lower limit (closed)
//         if(curtain->getCurrentYPos() > 0) {
//           curtain->setYPosClosed(curtain->getCurrentYPos());
//         }
//       }
//     } else {
//       //handling speed change request
//       int speed = std::stoi(value);
//       Serial.println("Speed change reqested:");
//       WebSerial.println("Speed change reqested:");
//       Serial.print(speed);
//       WebSerial.print(speed);
//       Serial.println(" rpm");
//       WebSerial.println(" rpm");

//       curtain->setStepperSpeed(speed);
//     }
//   }
// };

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
}


void setup() {
  curtain->setStepperSpeed(29);
  // initialize the serial port
  Serial.begin(115200);

  //wifi
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  //OTA starts
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //OTA ends

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(CHARACTERISTIC_2_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new RemoteCallback());
  pCharacteristic2->setCallbacks(new SetupCallback(curtain));

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();

  WebSerial.println("Setup complete");
}

void loop() {
  ArduinoOTA.handle();

  if(curtain->getRotorState() == RotorState::STOP) {
    //quit config mode whenever rotor stops
    if(curtain->getConfigMode()) {
      curtain->setConfigMode(false);
    }
    delay(100);
  } else if(curtain->getRotorState() == RotorState::UP) {
    //stop at limit, unless in config mode
    if(curtain->getCurrentYPos() > 0 || curtain->getConfigMode()) {
      curtain->incrementYPos(-STEP);
      curtain->stepperStep(-STEP);
    } else {
      curtain->resetCurrentYPos();
      curtain->setRotorState(RotorState::STOP);
    }
    WebSerial.println(curtain->getCurrentYPos());
    
  } else if(curtain->getRotorState() == RotorState::DOWN) {
    WebSerial.println(curtain->getCurrentYPos());
    curtain->incrementYPos(STEP);
    curtain->stepperStep(STEP);
  } else if(curtain->getRotorState() == RotorState::OPEN) {
    curtain->stepperStep(-curtain->getCurrentYPos());
    curtain->resetCurrentYPos();
  } else if(curtain->getRotorState() == RotorState::CLOSE) {
    curtain->stepperStep(curtain->getYPosClosed() - curtain->getCurrentYPos());
    curtain->setCurrentYPos(curtain->getYPosClosed());
  }
}