#include <Arduino.h>
#include <Stepper.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

const char* ssid = "Maszt 5G test 300% mocy";
const char* password = "aqq123321qqa";

const int stepsPerRevolution = 2048;
enum class RotorState { UP, STOP, DOWN };
RotorState rotorState = RotorState::STOP;

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

AsyncWebServer server(80);

#define SERVICE_UUID        "eda3620e-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_UUID "f67783e2-0e6a-11ed-861d-0242ac120002"
#define CHARACTERISTIC_2_UUID "2250634e-8aa7-4e3e-b3a1-31d4bbe40127"

class RemoteCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() == 1) {
      if(value == "U" && rotorState == RotorState::STOP) {
        rotorState = RotorState::UP;
        Serial.println("Going UP");
      } else if(value == "S" && rotorState != RotorState::STOP) {
        rotorState = RotorState::STOP;
        Serial.println("STOPPING");
      } else if(value == "D" && rotorState == RotorState::STOP) {
        rotorState = RotorState::DOWN;
        Serial.println("Going DOWN");
      }
    } else if (value.length() > 1) {
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++)
        Serial.print(value[i]);

      Serial.println();
      Serial.println("*********");
    }
  }
};

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

class SetupCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    int speed = std::stoi(value);
    Serial.println("Speed change reqested:");
    Serial.print(speed);
    Serial.println(" rpm");
    myStepper.setSpeed(speed);
  }
};

void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
}


void setup() {
  myStepper.setSpeed(25);
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
  pCharacteristic2->setCallbacks(new SetupCallback());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();

  Serial.println("Setup complete");
}

void loop() {
  ArduinoOTA.handle();

  if(rotorState == RotorState::STOP) {
    delay(100);
    WebSerial.println("Hello!");
  } else if(rotorState == RotorState::UP){
    myStepper.step(150);
  } else if(rotorState == RotorState::DOWN){
    myStepper.step(-150);
  }
}