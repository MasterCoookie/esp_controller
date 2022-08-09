#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Curtain.h"

class RemoteCallback: public BLECharacteristicCallbacks {
public:
    RemoteCallback(Curtain* curtain);
private:
    Curtain* curtain;
    void onWrite(BLECharacteristic *pCharacteristic);
};