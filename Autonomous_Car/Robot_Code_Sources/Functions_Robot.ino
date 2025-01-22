void ISR_countA()  
{
  counter_A++; 
} 
 

void ISR_countB()  
{
  counter_B++; 
}
 






void MoveForward(int steps, int mspeed){
   counter_A = 0;  
   counter_B = 0; 
   digitalWrite(in1, HIGH);
   digitalWrite(in2, LOW);
   analogWrite(enA,255);
   delay(10);


   while (steps > counter_A && steps > counter_B) {
   
    if (steps > counter_A) {
      analogWrite(enA, mspeed);
      } else {
      analogWrite(enA, 0);
      }
  // Stop when done
    analogWrite(enA, 0);
  
    Serial.print(mspeed);
    Serial.println(" ");
    counter_A = 0;  //  reset counter A to zero
    counter_B = 0;  //  reset counter B to zero 
   }
}
 








void MoveReverse(int steps, int mspeed) 
{
   counter_A = 0;  
   counter_B = 0; 
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
 
   

   while (steps > counter_A && steps > counter_B) {
   
    if (steps > counter_A) {
    analogWrite(enA, mspeed);
    } else {
    analogWrite(enA, 0);
    }
   
    }
  analogWrite(enA, 0);

  counter_A = 0;
  counter_B = 0;  
 
}










 
//// Function to Spin Right
//void SpinRight(int steps, int mspeed){
//   counter_A = 0;  //  reset counter A to zero
//   counter_B = 0;  //  reset counter B to zero
//    
//  // Stop when done
//  analogWrite(enA, 0);
//
//  counter_A = 0;  //  reset counter A to zero
//  counter_B = 0;  //  reset counter B to zero 
// 
//}
// 
//// Function to Spin Left
//void SpinLeft(int steps, int mspeed) 
//{
//   counter_A = 0;  //  reset counter A to zero
//   counter_B = 0;  //  reset counter B to zero
//   
//
//  counter_A = 0;  //  reset counter A to zero
//  counter_B = 0;  //  reset counter B to zero 
// 
//}
// 
