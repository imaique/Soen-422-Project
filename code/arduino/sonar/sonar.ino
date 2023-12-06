#include <ESP32Servo.h>
#include <NewPing.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>



const char* ssid     = "MJO";
const char* password = "michaelosuji";
// add refresh profile to accomodate for cases where we move the arduino

Servo myservo;  // create servo object to control a servo

int angle = 0;    // variable to store the servo position

int servoPin = 12;

const int maxRange = 50; // centimeters
const int minServoPosition = 0;
const int maxServoPosition = 180;

const float ERROR_FACTOR = 0.1;

const int profile_length = maxServoPosition - minServoPosition + 1;

int neutral [profile_length];

const int trigPin = 14;
const int echoPin = 27;
long duration;
int distance;

unsigned long lastPingTime = 0;
unsigned int pingPeriod = 50;

bool completedSweep = false;

const String addObjectEndpoint = "https://on-add-detected-object-cwle57ukha-uc.a.run.app/";
//const String addObjectEndpoint = "https://httpbin.org/response-headers";

const String token = "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU0YWRmYjQzNmI5ZTE5N2UyZTExMDZhZjJjODQyMjg0ZTQ5ODZhZmYiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJhY2NvdW50cy5nb29nbGUuY29tIiwiYXpwIjoiNjE4MTA0NzA4MDU0LTlyOXMxYzRhbGczNmVybGl1Y2hvOXQ1Mm4zMm42ZGdxLmFwcHMuZ29vZ2xldXNlcmNvbnRlbnQuY29tIiwiYXVkIjoiNjE4MTA0NzA4MDU0LTlyOXMxYzRhbGczNmVybGl1Y2hvOXQ1Mm4zMm42ZGdxLmFwcHMuZ29vZ2xldXNlcmNvbnRlbnQuY29tIiwic3ViIjoiMTA3NzUwNjM4NTUzMTEyNjc4MDM0IiwiZW1haWwiOiJvc3VqaW0yMDEyQGdtYWlsLmNvbSIsImVtYWlsX3ZlcmlmaWVkIjp0cnVlLCJhdF9oYXNoIjoibklJSlNEQ1lTbzU0WTd6OHdRNF9qdyIsIm5iZiI6MTcwMTgwNTM1MywiaWF0IjoxNzAxODA1NjUzLCJleHAiOjE3MDE4MDkyNTMsImp0aSI6IjVlNmRjMzcxOTdlNzc2MzY2YjcwMjZhNWExNGIzM2YxY2U3ZDEyNjMifQ.BBumAPX9ndleb7TbbS0M1mo9kou9eXo0PSG0jh1oGdaTSTIzYxcY-Dqguye0u-PpAve2wSEjehU-YzpXfipw6VwT7Fe3TvnU64OjYFof4fWICdwTPquzpWtdvYX-sylDV9n6eb-JBAyNJ-OqdRG_HJfupDQHnKLxzxe-fV-GLx2-RZ-YSAGL_h9cJ1r-zPoan3NpCjWVCxZQh1iTW94I4T3ZJcs97rTdnelAftrT_BXi1Xke6j-ZJBoraeIrRghUcqr20yFvFedHl4M6MXpLqafmg4_gJl8PMex5RFziu0FQEo-pOzoKYJGYeJYlwCDEkAxIwtvb9cvhYA38r69-zQ";

NewPing sonar(trigPin, echoPin);

HardwareSerial Sender(1);   // Define a Serial port instance called 'Sender' using serial port 1
#define Sender_Txd_pin 17
#define Sender_Rxd_pin 16

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected.");
}

void setupListenerSerial() {
  Sender.begin(115200, SERIAL_8N1, Sender_Txd_pin, Sender_Rxd_pin); // Define and start Sender serial port
}

void setupSonar() {
  Serial.println("Setting up Sonar.");
  //pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  //pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.println("Done setting up Sonar.");
}

void setupServo() {
  Serial.println("Setting up Servo.");
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); // attaches the servo to the servo object
  Serial.println("Done setting up Servo.");
}
void moveServo() {
  static bool rising = true;

  if(rising) {
    if(angle == maxServoPosition) {
      rising = false;
      myservo.write(--angle);
    } else {
      myservo.write(++angle);
    }
  } else {
    if(angle == minServoPosition) {
      rising = true;
      myservo.write(++angle);
    } else {
      myservo.write(--angle);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  setupServo();
  setupSonar();
  setupListenerSerial();
  connectToWiFi();
}

void notify_object(int angle1, int angle2, int distance) {
  int angle_start = min(angle1, angle2), angle_end = max(angle1, angle2);
  if(angle_end - angle_start < 3) return;
  if(WiFi.status() != WL_CONNECTED) connectToWiFi();
  Serial.println(WiFi.status() == WL_CONNECTED);
  WiFiClientSecure *client = new WiFiClientSecure;
  if(!client) {
    Serial.println("[HTTPS] Unable to connect");
    return;
  }
  client -> setInsecure();
  {
    HTTPClient http;

    String serverPath = addObjectEndpoint + "?start=" + String(angle_start) + "&end=" + String(angle_end) + "&distance=" + String(distance);
    
    // Your Domain name with URL path or IP address with path
    Serial.println(serverPath);
      http.setTimeout(20000);
    http.setConnectTimeout(20000);
    http.begin(*client, serverPath.c_str());
    
    // If you need Node-RED/server authentication, insert user and password below
    http.addHeader("Authorization", "Bearer " + token);
    //http.addHeader("Content-Length", "0");
    // Send HTTP POST request

    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println(payload);
      JSONVar response = JSON.parse(payload);
      if(response.hasOwnProperty("refresh")){
        // reset sweep
        for (int i = 0; i < profile_length; i++) neutral[i] = 0;
        completedSweep = false;
      } else if(response.hasOwnProperty("object")) {
        // Forward payload to notifier
        Sender.print(payload);
        String name = response["name"];
        bool expected = (bool)response["expected"];
        
      } else {
        Serial.println("Response cannot be parsed");
      }
    }
    else {
      Serial.print("Unexpected code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
    delete client;

  Serial.println("Detected object from " + String(angle_start) + " to " + String(angle_end) + ", " + String(distance) + " cm away.");
}

bool same_distance(int base_distance, int new_distance) {
  const float difference = ERROR_FACTOR * maxRange;
  const float lower_bound = base_distance - difference;
  const float higher_bound = base_distance + difference;
  const bool same = lower_bound <= distance && distance <= higher_bound;
  return same;
}

void distanceUpdated(const int distance) {
  static int prev_angle = angle;
  const int base_distance = neutral[angle];

  if(completedSweep) {
    static int object_start = -1;
    static int object_dist = 0;

    const bool tracking_object = object_start != -1;
    Serial.print(tracking_object ? "TRACKING!" : "NOTHING ABNORMAL!");
    if(same_distance(base_distance, distance)) {
      // check if currently tracking object
      if(tracking_object) {
        notify_object(object_start, prev_angle, object_dist);

        // reset object tracking
        object_start = -1;
      }

    } else {
      if(tracking_object) {
        // if new object, send info about previous object
        if(!same_distance(object_dist, distance)) {
          notify_object(object_start, prev_angle, object_dist);
          object_start = angle;
          object_dist = distance;
        }
      } else {
          object_start = angle;
          object_dist = distance;
      }

    }
  } else {
    static int unsweptCount = profile_length;

    if(neutral[angle] == 0) {
      neutral[angle] = distance;
      unsweptCount--;
    }

    if(unsweptCount == 0) {
      completedSweep = true;
      sweepCompleted();
    }
  }
  prev_angle = angle;
}

void sweepCompleted() {
  Serial.println("Sweep Complete");
  for(int i = 0; i < profile_length; i++) {
    Serial.println("angle: " + String(i) + ", distance: " + String(neutral[i]));
  }
}
int calculateDistance(){ 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance= duration*0.034/2;
  return distance;
}

void updateDistance() {
  unsigned long currentTime = millis();
  if (currentTime - lastPingTime >= pingPeriod) {
    static int prev_reading = 0;
    lastPingTime = currentTime;
    // normalize distance
    distance = sonar.ping_cm();
    distance = min(distance, maxRange);
    if(prev_reading + 1 >= distance && distance >= prev_reading - 1) {
      Serial.println(distance);
      //if()
      distanceUpdated(distance);
      moveServo();
    }
    prev_reading = distance;

  }
}

void loop()
{
  updateDistance();
}