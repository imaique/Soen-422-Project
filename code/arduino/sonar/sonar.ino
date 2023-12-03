#include <ESP32Servo.h>
#include <NewPing.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>

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
  HTTPClient http;

  String serverPath = serverName + "?start=" + ;
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
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
  static bool completedSweep = false;
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