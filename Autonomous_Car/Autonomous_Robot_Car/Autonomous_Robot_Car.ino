
#include <SerialBT.h>
#include "tof.h"
#include "robotFunctions.h"



          
////////////PINS///////////////////////






int enA = 18;
int in1 = 20;
int in2 = 19;

//int SDA_PIN = 4;
//int SCL_PIN = 5;



///////////////VARIABLES//////////////////////////

int initialL=100;
int initialR=0;

// int laser_1;
// int laser_2;
// int laser_3;
// int laser_4;
// int laser_5;
// int laser_6;
// int laser_7;


char Incoming_value = 0;
String command;






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(encL,INPUT);
  pinMode(encR,INPUT);
  pinMode(enA,OUTPUT);
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(encL), ISR_countL, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encR), ISR_countR, CHANGE);
  tof_init();
  

  SerialBT.begin();

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop(){

  pwr_status(); 
  tof_display();

  

  //laser_4 = Sensor4.readRangeContinuousMillimeters();
  // Forward(130,"straight");
  // if (counter_L>=30 && counter_R>=30){//|| laser_4<=400 ){
    // Forward(0,"straight");
  // }

  // SerialBT.print("Laser Distance: ");
  // SerialBT.print(laser_4);
  // SerialBT.print("Left counter: ");
  // SerialBT.print(counter_L);
  // SerialBT.print(" Right counter: ");
  // SerialBT.println(counter_R);


  
  

  }
