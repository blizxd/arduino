#include <TimerOne.h>
#include <EEPROM.h>

#define LEN(x) (sizeof(x) / sizeof((x)[0]))

const int LEDS[] = {9, 10, 11, 12, 13};
const int LDR = A0;
const int BUTTON = 2;
const int BUZZER = 5;

const int MIN = 200;
const int MAX = 800;
const int STEP = (MAX - MIN) / 5;

const int ADDR_MAGIC     = 0;   
const int ADDR_THRESHOLD = 1;   
const int ADDR_MIN_LIGHT = 3;   
const int ADDR_MAX_LIGHT = 5;   
const byte MAGIC = 0x42;     

const unsigned long SAVE_INTERVAL = 60000UL;  

volatile bool sampleReady = false;
volatile bool buttonPressed = false;

int lightLevel = 0;
int threshold = (MAX + MIN) / 2;
int minLight = 1023;          
int maxLight = 0;
bool minMaxChanged = false;
unsigned long lastSave = 0;
bool alarmSounding = false;

void onTimer() {
  sampleReady = true;
}

void onButton() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 200) {
    buttonPressed = true;
    lastPress = now;
  }
}

void saveMinMax() {
  EEPROM.put(ADDR_MIN_LIGHT, minLight);
  EEPROM.put(ADDR_MAX_LIGHT, maxLight);
  minMaxChanged = false;
  lastSave = millis();
}

void loadSettings() {
  if (EEPROM.read(ADDR_MAGIC) != MAGIC) {

    EEPROM.put(ADDR_THRESHOLD, threshold);
    saveMinMax();
    EEPROM.update(ADDR_MAGIC, MAGIC);
    return;
  }

  EEPROM.get(ADDR_THRESHOLD, threshold);
  EEPROM.get(ADDR_MIN_LIGHT, minLight);
  EEPROM.get(ADDR_MAX_LIGHT, maxLight);


  if (threshold < MIN || threshold > MAX) threshold = (MAX + MIN) / 2;
  if (minLight < 0 || minLight > 1023) minLight = 1023;
  if (maxLight < 0 || maxLight > 1023) maxLight = 0;
}

void printStats() {
  Serial.print("current: ");
  Serial.print(lightLevel);
  Serial.print(" limit: ");
  Serial.print(threshold);
  Serial.print("  min light: ");
  Serial.print(minLight);
  Serial.print("  max ligth: ");
  Serial.println(maxLight);
}

void lightLEDs(int level) {
  int count = (long)(level - MIN) * LEN(LEDS) / (MAX - MIN);
  for (unsigned int i = 0; i < LEN(LEDS); ++i) {
    digitalWrite(LEDS[i], (int)i < count ? HIGH : LOW);
  }
}

void updateAlarm() {
  if (!alarmSounding && lightLevel > threshold) {
    alarmSounding = true;
    tone(BUZZER, 1000);                   
  } else if (alarmSounding && lightLevel < threshold - 20) {
    alarmSounding = false;
    noTone(BUZZER);
  }
}

void handleSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == 'p') {
    printStats();
  } else if (c == 'r') {
    minLight = 1023;
    maxLight = 0;
    saveMinMax();
    Serial.println("min/max reset");
  }
}

void setup() {
  for (unsigned int i = 0; i < LEN(LEDS); ++i) {
    pinMode(LEDS[i], OUTPUT);
  }
  Serial.begin(9600);

  loadSettings();
  printStats();
  Serial.println("send 'p' to print stats, 'r' to reset min/max");

  Timer1.initialize(100000);                  // 100 ms
  Timer1.attachInterrupt(onTimer);
  attachInterrupt(digitalPinToInterrupt(BUTTON), onButton, FALLING);
}

void loop() {
  if (sampleReady) {
    sampleReady = false;
    int raw = analogRead(LDR);

    if (raw < minLight) { minLight = raw; minMaxChanged = true; }
    if (raw > maxLight) { maxLight = raw; minMaxChanged = true; }

    lightLevel = constrain(raw, MIN, MAX);
    lightLEDs(lightLevel);
    updateAlarm();
  }

  if (buttonPressed) {
    buttonPressed = false;
    threshold = ((threshold + STEP - MIN) % (MAX - MIN)) + MIN;
    EEPROM.put(ADDR_THRESHOLD, threshold);
    Serial.print("Current limit: ");
    Serial.println(threshold);
  }

  if (minMaxChanged && millis() - lastSave >= SAVE_INTERVAL) {
    saveMinMax();
  }

  handleSerial();
}