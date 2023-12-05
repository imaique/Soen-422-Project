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

const String token = "eyJhbGciOiJSUzI1NiIsImtpZCI6ImU0YWRmYjQzNmI5ZTE5N2UyZTExMDZhZjJjODQyMjg0ZTQ5ODZhZmYiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJhY2NvdW50cy5nb29nbGUuY29tIiwiYXpwIjoiNjE4MTA0NzA4MDU0LTlyOXMxYzRhbGczNmVybGl1Y2hvOXQ1Mm4zMm42ZGdxLmFwcHMuZ29vZ2xldXNlcmNvbnRlbnQuY29tIiwiYXVkIjoiNjE4MTA0NzA4MDU0LTlyOXMxYzRhbGczNmVybGl1Y2hvOXQ1Mm4zMm42ZGdxLmFwcHMuZ29vZ2xldXNlcmNvbnRlbnQuY29tIiwic3ViIjoiMTA3NzUwNjM4NTUzMTEyNjc4MDM0IiwiZW1haWwiOiJvc3VqaW0yMDEyQGdtYWlsLmNvbSIsImVtYWlsX3ZlcmlmaWVkIjp0cnVlLCJhdF9oYXNoIjoidG8tdlFYQnRaT2lUMV9GcFJFbmh1QSIsIm5iZiI6MTcwMTYyMzkzNSwiaWF0IjoxNzAxNjI0MjM1LCJleHAiOjE3MDE2Mjc4MzUsImp0aSI6ImJmMmMxZTczMDJhNDhiMjM4YTE4NzAwY2RlYWFhYmY1YTQ4NWNjODUifQ.bloOr6wpl7_gljkKgCrURyKHPQY__tw4U0OmiAEDq5lItTrUvzuGZmLXWmBK5HE7suXoZLToTcl-u_-oo897MI6E8Q1H00ZhdFfMVPLu5D5uf5LxJphoICxCgWb2GnIpLhh1iNgza1MMir_c6-PoH7JcOrnzP6CW9k-GkAWQBXhdLLIQxSvlgnc1YAm9Nrrb5jAV-a7YD3_lFBMOIN80sdVnIEhYuf1Meg215mHOEBBe8RI6ZIJhP_0bgLbuAa91877zpJZ2lIie-FPlzmVhhlLTf7z7bCoWXSmRcGgqz5WFAmxAAusmnTEtCG7NzslR4p7PTL2lDWvVK8hnrS5ciA";

NewPing sonar(trigPin, echoPin);

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected.");
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
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      JSONVar response = JSON.parse(payload);
      if(response.hasOwnProperty("object")) {
        String name = response["name"];
        bool expected = (bool)response["expected"];
        
      } else if(response.hasOwnProperty("refresh")){
        // reset sweep
        for (int i = 0; i < profile_length; i++) neutral[i] = 0;
        completedSweep = false;
      } else {
        Serial.println("Response cannot be parsed");
      }
    }
    else {
      Serial.print("Error code: ");
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