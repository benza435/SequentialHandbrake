#include <HX711.h>


const byte SWITCH_PIN = 2;
const byte HX711_DAT_PIN = 14;
const byte HX711_CLK_PIN = 15;

volatile bool switchPressed = false;
HX711 scale;

void switchISR() {
  switchPressed = true;
}

void setup() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), switchISR, CHANGE);

  // Initialize HX711 library

  scale.begin(HX711_DAT_PIN, HX711_CLK_PIN);
}

void loop() {
  // Check switch state
  if (switchPressed) {
    Serial.println("==============================================================================  Switch pressed!");
    switchPressed = false;
  }

  // Read load cell data
  long reading = scale.read();
  Serial.print("Handbrake force: ");
  Serial.println(reading / 1000 - 22);
  
  delay(100);
}
