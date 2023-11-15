#include "BluetoothSerial.h"

String device_name = "ECHOES_Alarm";
BluetoothSerial SerialBT;

int buzzerPin = 21;

bool unknown_present = false;
bool known_present = false;

int unknowns[180];
int knowns[180];

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

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
}

int getNextInt() {
  return SerialBT.readStringUntil(',').toInt();
}

void react() {
  if(unknown_present) {
    alarm_manager.play();
  } else {
    alarm_manager.stop();
  }
}

void updateAlarm() {
  unknown_present = false;
  known_present = false;
  for(int d : unknowns) {
    if(d != 0) {
      unknown_present = true;
      break;
    }
  }
  for(int d : knowns) {
    if(d != 0) {
      known_present = true;
      break;
    }
  }
}

void loop() {
  react();
  if (SerialBT.available()) {
    int type = getNextInt(), distance = getNextInt(), location = getNextInt();

    switch(type) {
      case 0:
        knowns[location] = distance;
        break;
      case 1:
        unknowns[location] = distance;
        break;
      case 2:
        knowns[location] = 0;
        unknowns[location] = 0;
        break;      
    }
    updateAlarm();
    Serial.println(type);
    Serial.println(distance);
    Serial.println(location);
    Serial.println();
  }
  delay(20);
}
