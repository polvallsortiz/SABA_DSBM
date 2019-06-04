#include <Servo.h>

//Variables per els 2 servos 
Servo servoVertical,servoHorizontal;
int pinVertical = 2;
int pinHorizontal = 3; 

void setup() {
  // Iniciamos el monitor serie para mostrar el resultado
  Serial.begin(9600);
  servoVertical.attach(pinVertical);
  servoHorizontal.attach(pinHorizontal);

  servoVertical.write(90);
  servoHorizontal.write(0);
}

void loop() {
  for (int i = 0; i <= 180; i++)
  {
    servoHorizontal.write(i);
    delay(25);
  }

  for (int i = 179; i > 0; i--)
  {
    servoHorizontal.write(i);
    delay(25);
  }
}
