#include <Arduino.h>


#include <NimBLEDevice.h>
/*
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>*/
#include <Adafruit_NeoPixel.h>

// NEOPIXEL SETUP
#define PIN            22
#define NUMPIXELS      19
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);



BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool deviceWritten   = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
extern int _r, _g, _b;
class MyCallbacks: public BLECharacteristicCallbacks {
    
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++){
          Serial.print(rxValue[i]);
        }
        Serial.println();

        String command = rxValue.c_str();
        command = command.substring(0,2);
    
        if(command == "!C"){
          uint8_t r = (uint8_t)rxValue[2];
          uint8_t g = (uint8_t)rxValue[3];
          uint8_t b = (uint8_t)rxValue[4];

          Serial.printf("R: %d | G: %d | B: %d \n\r",r,g,b);

          for(int i=0;i<NUMPIXELS;i++){
            pixels.setPixelColor(i, pixels.Color(g,r,b));
            }
          pixels.show();

          //update AnimateRing
          _r = g; _g = r; _b = b;
          deviceWritten = true;
        }
        Serial.println(command);
        Serial.println("*********");
      }
    }
};


extern void setupAR();
extern void loopAR();


void setup() {
  Serial.begin(115200);

  // SETUP NeoPixel
  pixels.begin();

  // Create the BLE Device
  BLEDevice::init("NeoPixel Controller BLE");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      //BLECharacteristic::PROPERTY_NOTIFY
                      NIMBLE_PROPERTY::NOTIFY
                    );

  //pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         //BLECharacteristic::PROPERTY_WRITE
                                         NIMBLE_PROPERTY::WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  setupAR();
}

void loop() {

  //if (deviceConnected) {
  if (deviceWritten) {
    deviceWritten = false;
    //Serial.printf("*** Sent Value: %d ***\n", txValue);
    pCharacteristic->setValue(&txValue, 1);
    pCharacteristic->notify();
    delay(500);
    }

  loopAR();
}