// C++ code
//
#define LEN(x) sizeof(x)/sizeof(x[0])

//0 = top led
// going clockwise
const int anodes[] =   {9,10,9,11,9,12,10,11,10,12};
const int cathodes[] = {10,9,11,9,12,9,11,10,12,11};
const int leds[]={12,11,10,9};

const int BUZZER=3;
const int BUTTON=6;

const uint16_t melodyFreq[] = {
  0, 1319, 0, 1175, 0, 740, 0, 415, 0, 370, 330, 0,
  294, 330, 0, 440, 0, 554, 0, 659, 0, 880, 0,
};
const uint16_t melodyDur[] = {
  630, 136, 52, 81, 46, 177, 55, 223, 12, 139, 119, 15,
  197, 380, 44, 73, 44, 203, 17, 218, 32, 789, 1387,
};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON,INPUT);
  Serial.begin(9600);
}

void clearAll(){
  
  for(int i=0;i< LEN(leds);++i){
    pinMode(leds[i],INPUT);
  }
}


void setLED(int i){
 pinMode(anodes[i], OUTPUT);
 digitalWrite(anodes[i],HIGH);
 pinMode(cathodes[i], OUTPUT); 
 digitalWrite(cathodes[i],LOW); 
}
enum State { INACTIVE, RUNNING, ALARM };

State currentState = INACTIVE;
bool currentButtonState = false;
bool prevButtonState = false;
int currentLED = 0;
int counter =0;
const int MAX_TICKS=200;
void playMelody(){
  
  for (int i=0;i<LEN(melodyDur);++i){
      if (melodyFreq[i] > 0) tone(BUZZER, melodyFreq[i]);
   	  else       noTone(BUZZER);
      delay(melodyDur[i]);
  }
}

void updateLEDS(){
  clearAll();
  counter +=1;
  currentLED = (counter / 20)%10;
  setLED(currentLED);
  delay(50);
}


bool checkButton(bool currentState){
  if (digitalRead(BUTTON)!=currentState){
  	 delay(10);
     return digitalRead(BUTTON);
  }
  return currentState; 	
}



void loop()
{
  currentButtonState = checkButton(currentButtonState);
	
  bool buttonPressed=(currentButtonState==HIGH 
                      && prevButtonState==LOW);
  
  prevButtonState = currentButtonState;
  
  switch(currentState){
  	case INACTIVE:
    	if(buttonPressed) currentState=RUNNING;
    
    break;
    case RUNNING:
    	if(buttonPressed) {
          currentState=INACTIVE;
          clearAll();
          counter=0;
          break;
        }
    	if(counter==MAX_TICKS)
        {
          currentState=ALARM;
          clearAll();
          counter=0;
          break;}
    	updateLEDS();
    break;
    case ALARM:
    	playMelody();
    	currentState=INACTIVE;
    break;
  }

  
}