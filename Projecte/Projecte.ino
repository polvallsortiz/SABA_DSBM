#define STATE_STANDBY 0
#define STATE_CONTROL 1
#define STATE_ALARMA 2


#include <Servo.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "Timer.h"
#include "RTClib.h"

//RTC Variable
RTC_DS1307 RTC;
struct Start {
  int hour;
  int minute;
};

//Variables per els 2 servos
Servo servoVertical, servoHorizontal;
int pinVertical = 6;
int pinHorizontal = 7;
int pinTrig = 9;
int pinEcho = 10;

long duration;
int distance;

int servoVerticalPos = 0;
int servoHorizontalPos = 0;

//Bluetooth mòdul
SoftwareSerial BTserial(5, 4); // RX | TX

//LEDS
int pinVerd = 11;
int pinVermell = 12;

//POLSADOR
int pinButton = 2;
int numInterrupt = 0;

//ARRAY
int distancesLast[32];
int distances[32];
bool first;

//VARIABLES
int pos;
int state;
Timer t;
int ledEvent;
Start start;

void setup() {
  Serial.begin(9600);

  //RTC MANAGEMENT
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
  }

  //DISTANCE CALCULATOR MANGEMENT
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);

  //SERVO MANAGEMENT
  servoVertical.attach(pinVertical);
  servoHorizontal.attach(pinHorizontal);
  servoVertical.write(45);
  servoHorizontal.write(0);
  
  //BLUETOOTH MANAGEMENT
  Serial.println("Enter AT commands:");
  BTserial.begin(9600);

  pinMode(pinVerd, OUTPUT);
  pinMode(pinVermell, OUTPUT);

  //STATE MANAGEMENT
  pinMode(pinButton, INPUT);
  changeState(STATE_STANDBY);
  first = true;
  
  //INTERRUPT INIT
  attachInterrupt(numInterrupt, pushButton, RISING);

  //POSITION MANAGEMENT
  pos = 5;
}

void loop() {
  t.update();
  comprovarBluetooth();
  comprovarTimer();
  if (state == STATE_CONTROL) {
    servoHorizontal.write(pos);
    if (pos >= 10 && pos < 170) {
      distance = ping(pinTrig, pinEcho);
      distances[(pos / 5 - 2)] = distance;
    }
    if (pos >= 180) {
      if (!first) {
        compareMeans();
        changeLast();
      }
      else {
        changeLast();
        first = false;
      }
      pos = 0;
      servoHorizontal.write(pos);
    }
    else pos += 5;
  }
}

int ping(int TriggerPin, int EchoPin) {
  float duration, distanceCm;

  digitalWrite(TriggerPin, LOW);  //To stabilize the pulse pin to LOW 4micros
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);  //Shot
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);  //Finalize shot

  duration = pulseIn(EchoPin, HIGH, 2000000);  //Time in microseconds of response

  if (duration < 25000000) {
    float delayCalc = 25000000 - duration;
    delayMicroseconds(int(delayCalc));
  }

  distanceCm = duration * 10 / 292 / 2;  //convertimos a distancia, en cm
  return distanceCm;
}

void pushButton() {
  Serial.write("PUSH\n");
  bool correct = true;
  int i = 0;
  while (i < 10 && correct) {
    delay(2000);
    int currentState = digitalRead(pinButton);
    if (currentState == 0) correct = false;
    ++i;
  }
  if (correct) {
    if(state == STATE_STANDBY) {
      first = true;
      changeState(STATE_CONTROL);
    }
    else {
      changeState(STATE_STANDBY);
    }
  }
}

void compareMeans() {
  Serial.println("A compare");
  int avLast = 0;

  for (int i = 0; i < 32; ++i) {
    avLast += distancesLast[i];
  }
  int meanLast = avLast / 32;
  int av = 0;
  for (int j = 0; j < 32; ++j) {
    av += distances[j];
  }
  int mean = av / 32;
  Serial.println(abs(mean - meanLast));
  if (abs((mean - meanLast)) > 50) {
    changeState(STATE_ALARMA);
  }
}

void changeLast() {
  for (int i = 0; i < 32; ++i) {
    distancesLast[i] = distances[i];
  }
}

bool compareLast(int pos, int value) {
  const float tolerancia = 0.5;
  return (distances[pos] * (1 + tolerancia) > value && distances[pos] * (1 - tolerancia) < value) || abs(distances[pos] - value) < 50;
}

void comprovarBluetooth() {
  if (BTserial.available()) {
    //Serial.write(BTserial.read());
    String receivedString = BTserial.readString();
    Serial.println(receivedString);
    if (receivedString == "H") {
      changeState(STATE_CONTROL);
      first = true;
    }
    else if (receivedString == "L") {
      changeState(STATE_STANDBY);
    }
    else {
      Serial.println(receivedString);
      String hourt = getSubstring(receivedString, '-', 0);
      String minutes = getSubstring(receivedString, '-', 1);
      start.hour = hourt.toInt();
      start.minute = minutes.toInt();
    }
  }
}

void comprovarTimer() {
  DateTime nowtime = RTC.now();
  if (nowtime.hour() == start.hour && nowtime.minute() == start.minute && state != STATE_CONTROL) {
    changeState(STATE_CONTROL);
  }
}

void changeState(int newState) {
  state = newState;
  switch (newState) {
    case STATE_STANDBY:
      t.stop(ledEvent);
      servoHorizontal.write(0);
      digitalWrite(pinVermell, LOW);
      digitalWrite(pinVerd, HIGH);
      break;
    
    case STATE_CONTROL:
      digitalWrite(pinVermell, LOW);
      ledEvent = t.oscillate(pinVerd, 100, HIGH);
      break;
      
    case STATE_ALARMA:
      t.stop(ledEvent);
      digitalWrite(pinVermell, HIGH);
      digitalWrite(pinVerd, LOW);
      break;
  }
}

String getSubstring(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

