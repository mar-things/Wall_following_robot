#include "tof.h"
#include <Wire.h>
#include <VL53L0X.h>

//#define XSHUT_pin5 not required for address change
#define XSHUT_pin6 11
#define XSHUT_pin5 10
#define XSHUT_pin4 9
#define XSHUT_pin3 8
#define XSHUT_pin2 7
#define XSHUT_pin1 6

//ADDRESS_DEFAULT 0b0101001 or 41
//#define Sensor1_newAddress 41 not required address change
#define Sensor2_newAddress 42
#define Sensor3_newAddress 43
#define Sensor4_newAddress 44
#define Sensor5_newAddress 45
#define Sensor6_newAddress 46
#define Sensor7_newAddress 47


VL53L0X Sensor1;
VL53L0X Sensor2;
VL53L0X Sensor3;
VL53L0X Sensor4;
VL53L0X Sensor5;
VL53L0X Sensor6;
VL53L0X Sensor7;

int laser_1;
int laser_2;
int laser_3;
int laser_4;
int laser_5;
int laser_6;
int laser_7;

void tof_init(){ /*WARNING*/
  //Shutdown pins of VL53L0X ACTIVE-LOW-ONLY NO TOLERANT TO 5V will fry them
  pinMode(XSHUT_pin1, OUTPUT);
  pinMode(XSHUT_pin2, OUTPUT);
  pinMode(XSHUT_pin3, OUTPUT);
  pinMode(XSHUT_pin4, OUTPUT);
  pinMode(XSHUT_pin5, OUTPUT);
  pinMode(XSHUT_pin6, OUTPUT);
  
  Serial.begin(9600);
  
  Wire.begin();
  //Change address of sensor and power up next one
  Sensor7.setAddress(Sensor7_newAddress);
  pinMode(XSHUT_pin6, INPUT);
  delay(10); //For power-up procedure t-boot max 1.2ms "Datasheet: 2.9 Power sequence"
  Sensor6.setAddress(Sensor6_newAddress);
  pinMode(XSHUT_pin5, INPUT);
  delay(10); //For power-up procedure t-boot max 1.2ms "Datasheet: 2.9 Power sequence"
  Sensor5.setAddress(Sensor5_newAddress);
  pinMode(XSHUT_pin4, INPUT);
  delay(10);
  Sensor4.setAddress(Sensor4_newAddress);
  pinMode(XSHUT_pin3, INPUT);
  delay(10);
  Sensor3.setAddress(Sensor3_newAddress);
  pinMode(XSHUT_pin2, INPUT);
  delay(10);
  Sensor2.setAddress(Sensor2_newAddress);
  pinMode(XSHUT_pin1, INPUT);
  delay(10);
  
  Sensor1.init();
  Sensor2.init();
  Sensor3.init();
  Sensor4.init();
  Sensor5.init();
  Sensor6.init();
  Sensor7.init();
  
  Sensor1.setTimeout(500);
  Sensor2.setTimeout(500);
  Sensor3.setTimeout(500);
  Sensor4.setTimeout(500);
  Sensor5.setTimeout(500);
  Sensor6.setTimeout(500);
  Sensor7.setTimeout(500);

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  Sensor1.startContinuous();
  Sensor2.startContinuous();
  Sensor3.startContinuous();
  Sensor4.startContinuous();
  Sensor5.startContinuous();
  Sensor6.startContinuous();
  Sensor7.startContinuous();

}

void tof_display(){ 
  
  laser_1 = Sensor1.readRangeContinuousMillimeters();
  laser_2 = Sensor2.readRangeContinuousMillimeters();
  laser_3 = Sensor3.readRangeContinuousMillimeters();
  laser_4 = Sensor4.readRangeContinuousMillimeters();
  laser_5 = Sensor5.readRangeContinuousMillimeters();
  laser_6 = Sensor6.readRangeContinuousMillimeters();
  laser_7 = Sensor7.readRangeContinuousMillimeters();
  
  //Serial.print(" Sensor 1: ");
  Serial.print(Sensor1.readRangeContinuousMillimeters());
  Serial.print(',');
  //Serial.print(" Sensor 2: ");
  Serial.print(Sensor2.readRangeContinuousMillimeters());
  Serial.print(',');
  //Serial.print(" Sensor 3: ");
  Serial.print(Sensor3.readRangeContinuousMillimeters());
  Serial.print(',');
  //Serial.print(" Sensor 4: ");
  Serial.print(Sensor4.readRangeContinuousMillimeters());
  Serial.print(',');
  //Serial.print(" Sensor 5: ");
  Serial.print(Sensor5.readRangeContinuousMillimeters());
  Serial.print(',');
  //Serial.print(" Sensor 6: ");
  Serial.print(Sensor6.readRangeContinuousMillimeters());
  Serial.print(',');
  //Serial.print(" Sensor 7: ");
  Serial.print(Sensor7.readRangeContinuousMillimeters());
  Serial.println();



  
}