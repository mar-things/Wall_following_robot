
  
  int stateLast;
  int stateCurrent;
  int rotationCount = 0;
  int stateCount = 0;
  int stateCountTotal = 0;
  int statesPerRotation = 6;
  int distancePerStep;
  int wDiameter = 77; // Wheel Diameter in mm
  int wdistance =0;


  
  int encL=26;

  int servoM = 28;
  int enA = 18;
  int in1 = 20;
  int in2 = 19;

void Forward();
void Backward();
    
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(encL,INPUT);
  pinMode(enA,OUTPUT);
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(servoM,OUTPUT);
  Serial.begin(115200);
 
 
  
 

}

void loop() {


  Forward();
  analogWrite(enA,90);
  analogWrite(servoM,90);
    
  stateLast = digitalRead(encL);
  distancePerStep = wDiameter/statesPerRotation;
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
   
   
    wdistance = distancePerStep*stateCountTotal;

    if (wdistance>=3000){
      analogWrite(enA,0);
    }
    Serial.print("distance: ");
    Serial.println(wdistance);
    

}
