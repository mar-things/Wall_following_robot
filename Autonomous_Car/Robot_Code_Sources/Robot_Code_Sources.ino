
 
const byte encR = 17;  
const byte encL = 16;  
const float stepcount = 12.00;  // Circle Segments
const float wheeldiameter = 77.98; 

volatile int counter_A = 0;
volatile int counter_B = 0;
 
int enA = 18;
int in1 = 20;
int in2 = 19;
 
// Interrupt Service Routines
void ISR_countA();
void ISR_countB();

//int MMtoSteps(float mm) {
// 
//  int result;  // Final calculation result
//  float circumference = (wheeldiameter * 3.14) ; // Calculate wheel circumference in mm
//  float mm_step = circumference / stepcount;  // MM per Step
//  
//  float f_result = mm / mm_step;  // Calculate result as a float
//  result = (int) f_result; // Convert to an integer (note this is NOT rounded)
//  
//  return result;  // End and return result
// 
//}
void MoveForward();
void MoveReverse();


void setup(){
  Serial.begin(9600);
  pinMode(encL,INPUT);
  pinMode(encR,INPUT);
  pinMode(enA,OUTPUT);
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(encL), ISR_countA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encR), ISR_countB, CHANGE);
  
  // Test Motor Movement  - Experiment with your own sequences here  
//  MoveForward(MMtoSteps(1000), 100);  // Forward half a metre at 255 speed
//  delay(1000);  // Wait one second

  
  
//  MoveReverse(10, 255);  // Reverse 10 steps at 255 speed
//  delay(1000);  // Wait one second
//  MoveForward(10, 150);  // Forward 10 steps at 150 speed
//  delay(1000);  // Wait one second
//  MoveReverse(MMtoSteps(25.4), 200);  // Reverse 25.4 cm at 200 speed
//  delay(1000);  // Wait one second
//  SpinRight(20, 255);  // Spin right 20 steps at 255 speed
//  delay(1000);  // Wait one second
//  SpinLeft(60, 175);  // Spin left 60 steps at 175 speed
//  delay(1000);  // Wait one second
//  MoveForward(1, 255);  // Forward 1 step at 255 speed

  
} 
 
 
void loop(){

  MoveForward(10, 180);  // Forward 10 steps at 150 speed
  delay(1000);  // Wait one second
 


}
