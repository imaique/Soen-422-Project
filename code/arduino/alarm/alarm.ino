/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
#include <Arduino_JSON.h>
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

int buzzerPin = 21;
const int green = 13;


class AlarmManager {
  private: 
    int buzzer_pin;
    const int frequency_duration = 10; // ms
    const int MAX_FREQ = 2000;
    const int MIN_FREQ = 1000;
    int current_frequency;
    bool playing = false;

    bool rising;
    unsigned long start_time = 0;

  public:
    AlarmManager(int buzzer_pin) : buzzer_pin(buzzer_pin)  {
      current_frequency = MIN_FREQ;
      rising = true;
    }
    void next_freq() {
      if(rising) {
        if(current_frequency == MAX_FREQ) {
          rising = false;
          current_frequency--;
        } else {
          current_frequency++;
        }
      } else {
        if(current_frequency == MIN_FREQ) {
          rising = true;
          current_frequency++;
        } else {
          current_frequency--;
        }
      }
    }
    void play(){
      playing = true;
      unsigned long current_time = millis();
      if(current_time - start_time >= frequency_duration) {
        start_time = current_time;
        tone(buzzer_pin, current_frequency);
        next_freq();
      }
    }
    void stop() {
      if(playing) {
        noTone(buzzer_pin);
        rising = true;
        current_frequency = 1000;
        playing = false;
      }
    }

};

AlarmManager alarm_manager (buzzerPin);

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.write(pData, length);
    Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  pinMode(buzzerPin, OUTPUT);
  pinMode(green, OUTPUT);


  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

static long lastUnexpectedTime = -10000000;
static const long ALARM_DURATION = 10000;

static long lastExpectedTime = -10000000;
static const long LIGHT_DURATION = 10000;

void processPayload(String payload) {
  JSONVar json_object = JSON.parse(payload);
  if(json_object.hasOwnProperty("object")) {
    bool expected = (bool)json_object["object"]["expected"];
    if(expected) {
      lastExpectedTime = millis();
      Serial.println("Detected expected object. Turning on light!");
    } else {
      lastUnexpectedTime = millis();
      Serial.println("Detected unexpected object. Sounding the alarm!");
    }
  }
}

void react() {
  long currentTime = millis();
  if(lastUnexpectedTime + ALARM_DURATION >= currentTime) {
    alarm_manager.play();
  } else {
    alarm_manager.stop();
  }

  if(lastExpectedTime + LIGHT_DURATION >= currentTime) {
    digitalWrite(green, HIGH);
  } else {
    digitalWrite(green, LOW);
  }
}

// This is the Arduino main loop function.
void loop() {
  react();

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  static long lastConnectionCheck = 0;
  static const int connectionRefresh = 100;

  const long currentTime = millis();
  if(currentTime >= lastConnectionCheck + connectionRefresh) {
    if (doConnect == true) {
      if (connectToServer()) {
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
    }

    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    if (connected) {
      static String prev_value = "";
      String current_value = pRemoteCharacteristic->readValue().c_str();
      if(prev_value != current_value) {
        processPayload(current_value);
        prev_value = current_value;
      }
    }else if(doScan){
      BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }
    lastConnectionCheck = currentTime;
  }
} // End of loop
