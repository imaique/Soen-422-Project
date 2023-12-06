/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

HardwareSerial Receiver(2); // Define a Serial port instance called 'Receiver' using serial port 2

#define Receiver_Txd_pin 17
#define Receiver_Rxd_pin 16

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Long name works now");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("{\"object\": {\"start\": 100, \"end\": 120, \"distance\": 50, \"expected\": true, \"timestamp\": \"2023-12-05 18:03:58.726798+00:00\", \"name\": \"unexpected\"}}");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}
unsigned long receiverChecked = 0;
const int receiverPeriod = 100;

unsigned long simulatedTime = 0;
const int simulatedPeriod = 8000;

const bool simulationON = true;

void loop() {
  if(receiverChecked + receiverPeriod < millis()) {
    receiverChecked = millis();
    if(Receiver.available()) {
      pCharacteristic->setValue(Receiver.readString().c_str());
    }
  }
  if(simulationON && simulatedTime + simulatedPeriod < millis()) {
    simulatedTime = millis();
    static bool expected = true;
    expected = !expected;
    String value = "{\"object\": {\"start\": 100, \"end\": 120, \"distance\": 50, \"expected\": " + String((expected ? "true" : "false")) + ", \"timestamp\": \"2023-12-05 18:03:58.726798+00:00\", \"name\": \"unexpected\"}}";
    pCharacteristic->setValue(value.c_str());
  }
}