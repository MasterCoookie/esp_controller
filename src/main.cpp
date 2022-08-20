#include <ArduinoOTA.h>

#include "EEPROM.h"
#include "SetupCallback.h"
#include "RemoteCallback.h"

#define EEPROM_SIZE 32
#define EEPROM_CHUNKS 4

//TMP

const char* password = "aqq123321qqa";

Curtain* curtain;

#define SERVICE_UUID          "eda3620e-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_UUID   "f67783e2-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_2_UUID "2250634e-8aa7-4e3e-b3a1-31d4bbe40127"

#define STEP 150

unsigned short int checkEventCoutner = 0;

unsigned short int currentEEPROMAddr = 0;

void EEPROMWrite(const char* data, unsigned short int& addr) {
  unsigned short int write_addr = addr;
  for (int i = 0; (i < EEPROM_SIZE) && (i < strlen(data)); ++i) {
        EEPROM.write(write_addr, data[i]);
        // Serial.print("Writing to: ");
        // Serial.print(write_addr);
        // Serial.print(" value: ");
        write_addr += 1;
        // Serial.println(char(EEPROM.read(write_addr)));
    }
    EEPROM.write(write_addr, 0);
    // Serial.print("Writing 0 to: ");
    // Serial.println(write_addr);
    addr += EEPROM_SIZE;
    EEPROM.commit();
}

String EEPROMRead(unsigned short int startingAddr) {
  String result = "";
  for (int i = startingAddr; i < (startingAddr + EEPROM_SIZE); ++i) {
        // Serial.print("Reading from: ");
        // Serial.print(i);
        byte readValue = EEPROM.read(i);
        // Serial.print(" read value: ");
        // Serial.println(char(readValue));
        if (readValue == 0) {
            break;
        }

        result += char(readValue);
    }
  //TMP
  // Serial.print("EEPRROM read val: ");
  // Serial.println(result);
  return result;
}

void setup() {
  // initialize the serial port
  Serial.begin(115200);
  delay(100);

  if (!EEPROM.begin(EEPROM_SIZE * EEPROM_CHUNKS)) {
      Serial.println("failed to init EEPROM");
  }

  // EEPROMWrite(ssid, currentEEPROMAddr);
  // EEPROMWrite(password, currentEEPROMAddr);

  String ssid = EEPROMRead(0);
  String password = EEPROMRead(EEPROM_SIZE);

  //wifi
  Serial.println("\nBooting");
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("Connected to ");
  Serial.println(ssid);

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

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //OTA ends
  
  //TIME STUFF
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600;
  const int   daylightOffset_sec = 3600;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  curtain = Curtain::getInstance();
  curtain->setStepperSpeed(29);

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(CHARACTERISTIC_2_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new RemoteCallback(curtain));
  pCharacteristic2->setCallbacks(new SetupCallback(curtain));

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("Setup complete");
}

void loop() {
  ArduinoOTA.handle();

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
    } else {
      
      checkEventCoutner = 0;
      curtain->checkPendingEvent();
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
  } else if(curtain->getRotorState() == RotorState::CLOSE) {
    curtain->stepperStep(curtain->getYPosClosed() - curtain->getCurrentYPos());
    curtain->setCurrentYPos(curtain->getYPosClosed());
  }
}