#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
#define SOIL_PIN 34
#define RAIN_PIN 35
#define FLOW_PIN 27
#define RELAY_PIN 26

// Thresholds (adjust if needed)
int soilThreshold = 1800;
int rainThreshold = 1500;

// Flow variables
volatile int pulseCount = 0;
float flowRate = 0;
unsigned long lastTime = 0;

// Interrupt for flow sensor
void IRAM_ATTR flowPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  // LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("SMART AGRO");

  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(2000);

  lcd.clear();

  // Pins
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Pump OFF

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(FLOW_PIN),
    flowPulse,
    RISING
  );
}

void loop() {

  // Read sensors
  int soilValue = analogRead(SOIL_PIN);
  int rainValue = analogRead(RAIN_PIN);

  // Convert to percentage
  int soilPercent = map(soilValue, 4095, 0, 0, 100);
  int rainPercent = map(rainValue, 4095, 0, 0, 100);

  // Flow calculation (every 1 second)
  if (millis() - lastTime >= 1000) {

    flowRate = pulseCount * 2.25;

    pulseCount = 0;

    lastTime = millis();
  }

  // Pump logic
  if (soilValue < soilThreshold && rainValue < rainThreshold) {

    digitalWrite(RELAY_PIN, LOW);  // ON

  } else {

    digitalWrite(RELAY_PIN, HIGH);  // OFF
  }

  // ------ LCD DISPLAY ------

  lcd.setCursor(0, 0);

  lcd.print("S:");
  lcd.print(soilPercent);
  lcd.print("% ");

  lcd.print("R:");
  lcd.print(rainPercent);
  lcd.print("%  ");  // clear

  lcd.setCursor(0, 1);

  lcd.print("F:");
  lcd.print(flowRate, 1);

  lcd.print("L ");

  if (digitalRead(RELAY_PIN) == LOW)
    lcd.print("P:ON ");

  else
    lcd.print("P:OFF");

  // ------ SERIAL ------

  Serial.println("----- STATUS -----");

  Serial.print("Soil: ");
  Serial.println(soilValue);

  Serial.print("Rain: ");
  Serial.println(rainValue);

  Serial.print("Flow: ");
  Serial.println(flowRate);

  // ------ ALERT ------

  if (digitalRead(RELAY_PIN) == LOW && flowRate < 1) {

    Serial.println("ALERT: No water flow!");
  }

  delay(1000);
}