#include "SetupCallback.h"

// void SetupCallback::onWrite(BLECharacteristic *pCharacteristic) {
//     std::string value = pCharacteristic->getValue();
//     if (value.length() == 1) {
//       configMode = true;
//       if(value == "U" && rotorState == RotorState::STOP) {
//         rotorState = RotorState::UP;
//       } else if(value == "S" && rotorState != RotorState::STOP) {
//         rotorState = RotorState::STOP;
//       } else if(value == "D" && rotorState == RotorState::STOP) {
//         rotorState = RotorState::DOWN;
//       } else if(value == "O") {
//         //set current pos as upper limit
//         currentYPos = 0;
//       } else if(value == "C") {
//         //set current pos as lower limit (closed)
//         if(currentYPos > 0) {
//           YPosClosed = currentYPos;
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

//       myStepper.setSpeed(speed);
//     }
// }