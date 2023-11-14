#include "WiFi.h"
#include "AsyncUDP.h"
#include <ESP32Servo.h>
#include <NewPing.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


const char * ssid = "MJO";
const char * password = "michaelosuji";

unsigned int localPort = 1234;      // local port to listen on

AsyncUDP udp;
Servo myservo;  // create servo object to control a servo

int pos = 16;    // variable to store the servo position

int servoPin = 18;

const int trigPin = 14;
const int echoPin = 27;
long duration;
int distance;

unsigned int pingTime = 50;

NewPing sonar(trigPin, echoPin);

void onPacket(AsyncUDPPacket packet) {
  // improve distance measurements
  Serial.print("Data: ");
  Serial.write(packet.data(), packet.length());
  Serial.println();
  //reply to the client
  AsyncUDPMessage udpMessage(8);
  udpMessage.write(distance);
  udpMessage.write(pos);
  Serial.println(distance);
  packet.send(udpMessage);
  //packet.printf("Got %u bytes of data", packet.length());
}

void setupWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    if(udp.listen(localPort)) {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket(onPacket);
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
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  Serial.begin(115200);
  setupWifi();
  setupSonar();
}

void updateDistance() {
    if (millis() >= pingTime) {
      distance = sonar.ping_cm();
      pingTime += 50;
    }
}

void loop()
{
  updateDistance();
  //Send broadcast every second
  if(!(millis() % 1000)) udp.broadcast("Anyone here?");
}