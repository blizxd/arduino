#include <Servo.h>
#include <LiquidCrystal.h>

#include <EEPROM.h>

Servo myServo;
LiquidCrystal lcd{2, 3, 4, 5, 6, 7};

const int SERVO_PIN = 9;
const int TRIG_PIN  = 10;
const int ECHO_PIN  = 8;

const int THRESHOLD    = 8;   
const int CLOSED_ANGLE = 0;
const int OPEN_ANGLE   = 90;

// EEPROM layout
const int  MAGIC_ADDR = 0;     
const int  COUNT_ADDR = 1;    
const byte MAGIC      = 0xA5;

bool isClosed = true;
unsigned long openCount = 0;

void myPinModePORTB(int pin, int val){
  pin -=8;
  if(val){
    DDRB |= (1 << pin); // write bit
  }
  else{
    DDRB &= ~(1 << pin); //clear bit 
  }
}

void myDigitalWritePORTB(int pin, int val){
  pin -=8;
  if(val){
    PORTB |= (1 << pin); // write bit
  }
  else{
    PORTB &= ~(1 << pin); //clear bit 
  }
}

int myDigitalReadPORTB(int pin){
  pin -=8;
  int res = PINB & (1<<pin);
  return res!=0;
}

void setup() {
  Serial.begin(9600);

  myPinModePORTB(ECHO_PIN, INPUT);
  myPinModePORTB(TRIG_PIN, OUTPUT);

  if (EEPROM.read(MAGIC_ADDR) != MAGIC) {
    openCount = 0;
    EEPROM.put(COUNT_ADDR, openCount);
    EEPROM.write(MAGIC_ADDR, MAGIC);
  } else {
    EEPROM.get(COUNT_ADDR, openCount);
  }

  myServo.attach(SERVO_PIN);
  myServo.write(CLOSED_ANGLE);

  lcd.begin(16, 2);

  delay(1000);
}

int readDistance() {
  myDigitalWritePORTB(TRIG_PIN, LOW);
  delayMicroseconds(2);
  myDigitalWritePORTB(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  myDigitalWritePORTB(TRIG_PIN, LOW);

  long durationUs = pulseIn(ECHO_PIN, HIGH, 30000);  
  if (durationUs == 0) return 999;                   

  return durationUs * 0.017;  // cm
}

void openBarrier() {
  Serial.println("Barrier opening");
  for (int angle = CLOSED_ANGLE; angle <= OPEN_ANGLE; angle++) {
    myServo.write(angle);
    delay(20);
  }
  isClosed = false;

  openCount++;
  EEPROM.put(COUNT_ADDR, openCount); 
}

void closeBarrier() {
  Serial.println("Barrier closing");
  for (int angle = OPEN_ANGLE; angle >= CLOSED_ANGLE; angle--) {
    myServo.write(angle);
    delay(20);
  }
  isClosed = true;
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd == "count") {
    Serial.print("Times opened: ");
    Serial.println(openCount);
  }
  else if (cmd == "reset") {
    openCount = 0;
    EEPROM.put(COUNT_ADDR, openCount);
  }
  else if (cmd == "dist") {
    Serial.print("Distance: ");
    Serial.print(readDistance());
    Serial.println(" cm");
  }
  else if (cmd == "help") {
    Serial.println("  count - show how many times barrier opened");
    Serial.println("  reset- reset counter to 0");
    Serial.println("  dist  -show current distance");
  }
  else {
    Serial.println("unknown command ");
  }
}

void handleSerial() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        processCommand(input);
        input = "";
      }
    } else if (input.length() < 20) {
      input += c;
    }
  }
}

void printDistance(int distance) {
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  if (distance == 999) {
    lcd.print("---");      
  } else {
    lcd.print(distance);
    lcd.print("cm");
  }
  lcd.print("      ");      
}
void loop() {
  handleSerial();

  int distance = readDistance();
  printDistance(distance);

  if (distance <= THRESHOLD && isClosed) {
    openBarrier();
  }
  else if (distance > THRESHOLD + 10 && !isClosed) {
    closeBarrier();
  }

  delay(500);
}