#include "WiFi.h"
#include "WiFiUdp.h"
#include <ESP32Servo.h>


const char * ssid = "MJO";
const char * password = "michaelosuji";

IPAddress targetAddress = IPAddress(172,25,144,1);      // local port to listen on
unsigned int targetPort = 1234;      // local port to listen on

WiFiUDP udp;
Servo myservo;  // create servo object to control a servo

int pos = 16;    // variable to store the servo position

int servoPin = 18;

const int trigPin = 14;
const int echoPin = 27;
long duration;
int distance;

void sendDistance() {
  // improve distance measurements
  if(!WiFi.isConnected()) {
    connectWiFi();
  }

  udp.beginPacket(targetAddress,targetPort);
  udp.write(distance);
  udp.write(pos);
  udp.endPacket();
  Serial.println(distance);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

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
  Serial.begin(115200);
  connectWiFi();
  setupSonar();
}

void updateDistance() {
    // Clears the trigPin

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance based on the speed of sound
  distance = duration * 0.034 / 2;
}
bool shouldSendDistance() {
  static int lastDistanceSent = 0;
  static const int distancePeriod = 100;

  const int currentTime = millis();
  if(currentTime - lastDistanceSent >= distancePeriod) {
    lastDistanceSent = currentTime;
    return true;
  }
  return false;
}

void loop()
{
  updateDistance();
  //Send broadcast every second
  if(shouldSendDistance()) sendDistance();
}