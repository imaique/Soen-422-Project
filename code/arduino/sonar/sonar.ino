#include <ESP32Servo.h>
#include <NewPing.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

Servo myservo;  // create servo object to control a servo

int servoPosition = 16;    // variable to store the servo position

int servoPin = 12;

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
	myservo.attach(servoPin, 1000, 2000); // attaches the servo to the servo object
}
void moveServo() {
  static bool rising = true;
  const int minServoPosition = 0;
  const int maxServoPosition = 180;

  if(rising) {
    if(servoPosition == maxServoPosition) {
      rising = false;
      myservo.write(--servoPosition);
    } else {
      myservo.write(++servoPosition);
    }
  } else {
    if(servoPosition == minServoPosition) {
      rising = true;
      myservo.write(++servoPosition);
    } else {
      myservo.write(--servoPosition);
    }
  }
}


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  Serial.begin(115200);
  setupServo();
  setupSonar();
}

void updateDistance() {
    if (millis() >= pingTime) {
      distance = sonar.ping_cm();
      pingTime += 50;
      Serial.print("da");
      Serial.print(distance);
      Serial.print(",");
      Serial.println(servoPosition);
      moveServo();
    }
}

void loop()
{
  updateDistance();
}