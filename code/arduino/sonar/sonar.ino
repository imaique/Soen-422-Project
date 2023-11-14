#include <ESP32Servo.h>
#include <NewPing.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

Servo myservo;  // create servo object to control a servo

int pos = 16;    // variable to store the servo position

int servoPin = 18;

const int trigPin = 14;
const int echoPin = 27;
long duration;
int distance;

unsigned int pingTime = 50;

NewPing sonar(trigPin, echoPin);

void setupSonar() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void setupServo() {
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  Serial.begin(115200);
  setupSonar();
}

void updateDistance() {
    if (millis() >= pingTime) {
      distance = sonar.ping_cm();
      pingTime += 50;
      Serial.print("da");
      Serial.print(distance);
      Serial.print(",");
      Serial.println(pos);
    }
}

void loop()
{
  updateDistance();
}