
void Counter() {
  
  int LState = digitalRead(encL);
  int RState = digitalRead(encR);
  
  Serial.print("Sensor state: ");
  Serial.print("L");
  Serial.print(LState);
  Serial.print(" ");
  Serial.print("R");
  Serial.print(RState);
  Serial.println(" ");

 //Counter
  if (LState==HIGH && in2==HIGH){
    
    initialL++;
    
  }else if ( LState==HIGH && in2==LOW){
    
    initialL--;
  }
  Serial.print("counter: ");
  Serial.println(initialL);
  delay(500);

}
