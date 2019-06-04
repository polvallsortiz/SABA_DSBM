int pinButton = 2;
int numInterrupt = 0;

//LEDS
int pinVerd = 11;
int pinVermell = 12;


bool state = false;
bool changed = false;


void setup() {
  Serial.begin(9600);  
  pinMode(pinButton,INPUT);
  pinMode(pinVerd,OUTPUT);
  pinMode(pinVermell,OUTPUT);
  digitalWrite(pinVermell, HIGH);
  digitalWrite(pinVerd, LOW);
  attachInterrupt(numInterrupt,pushButton,RISING);
}

void pushButton() {
  Serial.write("PUSH\n");
  bool correct = true;
  int i = 0;
  while(i < 10 && correct) {
    delay(2000);
    int currentState = digitalRead(pinButton);
    if(currentState == 0) correct = false;
    Serial.write("Iteració : ");
    Serial.write(i);
    Serial.write(" amb state = ");
    Serial.write(state);
    Serial.write("\n");
    ++i;
  }
  if(correct) {
    changed = true;
    state = !state;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(changed) {
    changed = false;
    if(state) {
      digitalWrite(pinVermell,LOW); 
      digitalWrite(pinVerd, HIGH); 
    }
    else {
      digitalWrite(pinVermell, HIGH); 
      digitalWrite(pinVerd, LOW);
    }
  }
}
