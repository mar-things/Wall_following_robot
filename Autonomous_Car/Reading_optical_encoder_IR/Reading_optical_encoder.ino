int sensorL=26;
int sensorR=27;
int initialL=0;

void setup() {
pinMode(sensorL,INPUT);
pinMode(sensorR,INPUT);
Serial.begin(9600);


}

void loop() {
  int LState = digitalRead(sensorL);
  int RState = digitalRead(sensorR);
  Serial.print("Sensor state: ");
  Serial.print("L");
  Serial.print(LState);
  Serial.print(" ");
  Serial.print("R");
  Serial.print(RState);
  Serial.print(" ");
  
  //Counter
  if (LState==HIGH){
    
    initialL++;
    
  }
  Serial.print("counter: ");
  Serial.println(initialL);
  delay(500);

}
