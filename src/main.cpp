#include <ArduinoOTA.h>

#include "SetupCallback.h"
#include "RemoteCallback.h"

Curtain* curtain;

#define SERVICE_UUID          "eda3620e-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_UUID   "f67783e2-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_2_UUID "2250634e-8aa7-4e3e-b3a1-31d4bbe40127"

#define STEP 150

unsigned short int checkEventCoutner = 0;
unsigned short int retryCounter = 0;

BLEServer* pServer;
BLEAdvertising* pAdvertising;

void setup() {
  // initialize the serial port
  Serial.begin(115200);
  delay(100);
  Serial.println("\nBooting");

  if (!EEPROM.begin(EEPROM_SIZE * EEPROM_CHUNKS)) {
      Serial.println("failed to init EEPROM");
  }

  curtain = Curtain::getInstance();

  // curtain->EEPROMWrite("Maszt 5G test 300% mocy", 0);
  // curtain->EEPROMWrite("aqq123321qqa", 32);


  String ssid = curtain->EEPROMRead(0);
  String password = curtain->EEPROMRead(EEPROM_SIZE);

  if(ssid.length() > 0 && password.length() > 0) {
    //wifi
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.waitForConnectResult() != WL_CONNECTED && retryCounter < 4) {
      Serial.print("Connection Failed! Retrying count ");
      Serial.println(++retryCounter);
      delay(5000);
      WiFi.begin(ssid.c_str(), password.c_str());
    }

    if(WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected to ");
      Serial.println(ssid);
      //TODO if not connected, init offline
      curtain->initializeOnline();
    }

      //OTA starts
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      }).onEnd([]() {
        Serial.println("\nEnd");
      }).onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      }).onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

    ArduinoOTA.begin();
    if(curtain->isInOnlineMode()) {

      Serial.println("Ready in online mode");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      //OTA ends
      
      //TIME STUFF
      const char* ntpServer = "pool.ntp.org";
      const long  gmtOffset_sec = 3600;
      const int   daylightOffset_sec = 3600;
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    } else {
      curtain->initializeOffline();
    }
  }

  BLEDevice::init("MyESP32");
  pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(CHARACTERISTIC_2_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new RemoteCallback(curtain));
  pCharacteristic2->setCallbacks(new SetupCallback(curtain));

  pService->start();

  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("Setup complete");
}

void loop() {
  if(WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }

  if(pServer->getConnectedCount() == 0) {
    pAdvertising->start();
  }

  if(curtain->getRotorState() == RotorState::STOP) {
    curtain->stepperPowerOff();
    //quit config mode whenever rotor stops
    if(curtain->getConfigMode()) {
      curtain->setConfigMode(false);
    }
    delay(100);
    
    //checking for events every 100 secs
    if(checkEventCoutner < 100) {
      ++checkEventCoutner;
    } else if(curtain->isInOnlineMode()) {
      if(WiFi.status() == WL_CONNECTED) {
        curtain->checkPendingEvent();
      } else {
        WiFi.disconnect();
        WiFi.reconnect();
      }
      checkEventCoutner = 0;
    } else {
      //ESP.restart();
    }

  } else if(curtain->getRotorState() == RotorState::UP) {
    //stop at limit, unless in config mode
    if(curtain->getCurrentYPos() > 0 || curtain->getConfigMode()) {
      curtain->incrementYPos(-STEP);
      curtain->stepperStep(-STEP);
    } else {
      curtain->resetCurrentYPos();
      curtain->setRotorState(RotorState::STOP);
      curtain->stepperPowerOff();
    }
    
  } else if(curtain->getRotorState() == RotorState::DOWN) {
    curtain->incrementYPos(STEP);
    curtain->stepperStep(STEP);
  } else if(curtain->getRotorState() == RotorState::OPEN) {
    curtain->stepperStep(-curtain->getCurrentYPos());
    curtain->resetCurrentYPos();
    curtain->setRotorState(RotorState::STOP);
  } else if(curtain->getRotorState() == RotorState::CLOSE) {
    curtain->stepperStep(curtain->getYPosClosed() - curtain->getCurrentYPos());
    curtain->setCurrentYPos(curtain->getYPosClosed());
    curtain->setRotorState(RotorState::STOP);
  }
}