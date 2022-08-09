#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Curtain.h"

class SetupCallback: public BLECharacteristicCallbacks {
public:
    SetupCallback(Curtain* curtain);
private:
    Curtain* curtain;
    void onWrite(BLECharacteristic *pCharacteristic);
};