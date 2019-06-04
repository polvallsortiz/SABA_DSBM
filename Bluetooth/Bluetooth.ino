
   //Hem rebut un missatge del Bluetooth
   if (BTserial.available()) {  
        //Serial.write(BTserial.read());
        char receivedChar = BTserial.read();
        Serial.write(receivedChar);
        switch(receivedChar) {
          case 'w':
            servoVerticalPos += 10;
            activate = true;
            break;
           case 's':
            servoVerticalPos -= 10;
            activate = true;
            break;
           case 'a':
            servoHorizontalPos -= 10;
            activate = true;
            break;
           case 'd':
            servoHorizontalPos += 10;  
            activate = true;     
            break; 
       }
    }
