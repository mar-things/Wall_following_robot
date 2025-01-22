#include "robotFunctions.h"
#include <Servo.h>

Servo servoM;

int enA = 18;
int in1 = 20;
int in2 = 19;

void pwr_status() {

  int LEDState = HIGH;
  digitalWrite(LED_BUILTIN, LEDState);
  const long onDuration = 1000;
  const long offDuration = 1000;
  long rememberTime=0;
  
  if(LEDState == HIGH){
    if((millis()-rememberTime) >= onDuration){
      LEDState = LOW;
      rememberTime=millis();
    }
  }else{
    if((millis()-rememberTime) >= offDuration){
      LEDState = HIGH;
      rememberTime=millis();
    }
  }
digitalWrite(LED_BUILTIN, LEDState);
}



void ISR_countL()  
{
  int encL=26;
  volatile int counter_L = 0;
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 100) 
  {
    counter_L++;
  }
  last_interrupt_time = interrupt_time;
   
} 
 

void ISR_countR()  
{
  int encR=27;
  volatile int counter_R = 0;
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 100) 
  {
    counter_R++;
  }
  last_interrupt_time = interrupt_time;
}


void Backward(int speed,String dir) {
  
        
    analogWrite(enA,speed);
    digitalWrite(in1,HIGH);
    digitalWrite(in2,LOW);
    
    if (dir == "left"){
      servoM.write(120);
    }else if (dir == "right"){
      servoM.write(10);
    }else if(dir == "straight"){
      servoM.write(90);
    }
    
}

void Forward(int speed,String dir) {
  servoM.attach(28);
    analogWrite(enA,speed);
    digitalWrite(in1,LOW);
    digitalWrite(in2,HIGH);
    
      if (dir == "left"){
      servoM.write(120);
    }else if (dir == "right"){
      servoM.write(10);
    }else if (dir == "straight"){
      servoM.write(90);
    }
}







void Counter() {
  
  int stateLast ;//= digitalRead(encL);
  int stateCurrent;
  int rotationCount = 0;
  int stateCount = 0;
  int stateCountTotal = 0;
  int statesPerRotation = 7;
  int distancePerStep;
  int wDiameter = 77; // Wheel Diameter in mm
  int wdistance;
  

  distancePerStep = wDiameter/statesPerRotation;
  
   //Counter
   while(1){
    stateCurrent = digitalRead(encL);
    
      if (stateCurrent != stateLast){
        stateLast = stateCurrent;
        stateCount++;
        stateCountTotal++;
      }
      if (stateCount == statesPerRotation){
        rotationCount++;
        stateCount = 0;
      }
   }
  
    wdistance = distancePerStep*stateCountTotal;
    Serial.print("distance: ");
    Serial.println(wdistance);
}