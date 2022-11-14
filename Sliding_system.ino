#include <Servo.h>
#include <LiquidCrystal.h>
//button wiring
#define MissionButton         2
//resistor wiring
#define RessistorIN           A0
//servo motor wiring
#define ServoSig              3
//stepper motor wiring
#define StepperStep           4
#define StepperDir            5
//DC motor wiring
#define DCLead1               6
#define DCLead2               13
//lcd wiring
#define en                    11
#define rs                    12
#define d4                    10
#define d5                    9
#define d6                    8
#define d7                    7

#define Forward               1
#define Back                  0
#define stepperSpeed          500
#define Steps_to_from_BoxA    1400
#define Steps_to_from_BoxB    (Steps_to_from_BoxA*2)
#define Steps_to_from_BoxC    (Steps_to_from_BoxA*3)
#define Steps_to_from_BoxD    (Steps_to_from_BoxA*4)
#define Steps_to_from_BoxE    (Steps_to_from_BoxA*5)

#define BoxA_RES              100
#define BoxB_RES              (BoxA_RES*2)
#define BoxC_RES              (BoxA_RES*3)
#define BoxD_RES              (BoxA_RES*4)
#define BoxE_RES              (BoxA_RES*5)

#define DCTravellingTime      2000
#define ServoSpeed            5

#define debounceDelay         50

enum Boxes{NoMove, BoxA, BoxB, BoxC, BoxD, BoxE};
//Functions prototypes               
void Fire_Stepper(uint8_t boxNum);
void DC_Move(uint8_t dir);
void Flip_in_Box();

//global variabls 
uint16_t buttonState;             
uint16_t lastButtonState = LOW;   
unsigned long lastDebounceTime = 0;  
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); 
Servo myservo;

#define debug
float Vin = 5;            
float Vout = 0;          
float Rref = 992;          
float R = 0; 

void setup() 
{
  #ifdef debug
  Serial.begin(9600);
  #endif
  lcd.begin(16,2);
  myservo.attach(ServoSig); 
  pinMode((StepperStep,StepperDir), OUTPUT); 
  pinMode((DCLead1,DCLead2), OUTPUT);
  pinMode(MissionButton,INPUT);
  digitalWrite((StepperStep,StepperDir), LOW);
  digitalWrite((DCLead1,DCLead2), LOW);
}

void loop() {
  #if   0
  lcd.clear();
  lcd.print("System is Ready");
  #ifdef debug
  Serial.println("System is Ready");
  #endif
  delay(50);
  uint16_t reading = digitalRead(MissionButton);
        #ifdef debug
        Serial.print("Button state = ");
        Serial.print(reading);
        Serial.println("");
        #endif
  if (reading != lastButtonState) 
    lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) 
  {
    if (reading != buttonState) 
    {
      buttonState = reading;
      if (buttonState == HIGH) 
      {
        lcd.clear();
        lcd.print("In progress");
        uint16_t ReadResistor = analogRead(RessistorIN);  
        Vout = (Vin * ReadResistor) / 1023;    
        R = Rref * (1 / ((Vin / Vout) - 1)); 
        #ifdef debug
        Serial.print("Resistor value = ");
        Serial.print(ReadResistor);
        Serial.println("");
        #endif
        #define RES_Offset      100
        
        if(ReadResistor >= BoxA_RES-RES_Offset && ReadResistor <= BoxA_RES+RES_Offset)
          Fire_Stepper(BoxA);
        else if(ReadResistor >= BoxB_RES-RES_Offset && ReadResistor <= BoxB_RES+RES_Offset)
          Fire_Stepper(BoxB);
        else if(ReadResistor >= BoxC_RES-RES_Offset && ReadResistor <= BoxC_RES+RES_Offset)
          Fire_Stepper(BoxC);
        else if(ReadResistor >= BoxD_RES-RES_Offset && ReadResistor <= BoxD_RES+RES_Offset)
          Fire_Stepper(BoxD);
        else if(ReadResistor >= BoxE_RES-RES_Offset && ReadResistor <= BoxE_RES+RES_Offset)
          Fire_Stepper(BoxE);
         else
          Fire_Stepper(NoMove);
        
        lcd.clear();
        lcd.print("Mission Completed");
        #ifdef debug
        Serial.println("Mission Completed");
        #endif
        delay(1000);
      }
    }
  }
  lastButtonState = reading;
  #endif
}

void Fire_Stepper(uint8_t boxNum)
{
  #ifdef debug
  Serial.println("stepper is fired");
  #endif
  uint32_t i, travellingSteps = 0;
  DC_Move(Forward);
  digitalWrite(StepperDir, LOW);
  switch(boxNum)
  {
    case BoxA:travellingSteps = Steps_to_from_BoxA;break;
    case BoxB:travellingSteps = Steps_to_from_BoxB;break;
    case BoxC:travellingSteps = Steps_to_from_BoxC;break;
    case BoxD:travellingSteps = Steps_to_from_BoxD;break;
    case BoxE:travellingSteps = Steps_to_from_BoxE;break;
    default :travellingSteps = 0;break;
  }
  for(i = 0; i < travellingSteps; i++)
  {
    digitalWrite(StepperStep, HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(StepperStep, LOW);
    delayMicroseconds(stepperSpeed);
  }
  Flip_in_Box();
  
  digitalWrite(StepperDir, HIGH);
  for(i = 0; i < travellingSteps; i++)
  {
    digitalWrite(StepperStep, HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(StepperStep, LOW);
    delayMicroseconds(stepperSpeed);
  }
  DC_Move(Back);
  #ifdef debug
  Serial.println("stepper is off");
  #endif
}

void DC_Move(uint8_t dir)
{
  #ifdef debug
  Serial.println("DC is fired");
  #endif
  if(dir == Forward)
  {
    digitalWrite(DCLead1,HIGH);
    digitalWrite(DCLead2,LOW);
  }
  else
  {
    digitalWrite(DCLead2,HIGH);
    digitalWrite(DCLead1,LOW);
  }
  delayMicroseconds(DCTravellingTime);
  {
    digitalWrite(DCLead1,LOW);
    digitalWrite(DCLead2,LOW);
  }
  #ifdef debug
  Serial.println("DC is off");
  #endif
}

void Flip_in_Box()
{
  #ifdef debug
  Serial.println("servo is fired");
  #endif
  int16_t pos;
  #define ServoThreeshold         180
  for (pos = 0; pos <= ServoThreeshold; pos++) 
  { 
    myservo.write(pos);              
    delay(ServoSpeed);                       
  }
  delay(2000);
  for (; pos >= 0; pos--) 
  { 
    myservo.write(pos);              
    delay(ServoSpeed);                       
  }
  #ifdef debug
  Serial.println("servo is off");
  #endif
}
