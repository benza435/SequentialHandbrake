#include <Joystick.h>
#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif


// Last state of the buttons
int lastButtonState[2] = { 0, 0 };
int buttonMap[2] = { 3, 2 };



//pins for load cell
const int HX711_dout = 14;  //mcu > HX711 dout pin
const int HX711_sck = 15;   //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

// joystick setup
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   2, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering

void setup() {
  // Initialize Button Pins
  pinMode(2, INPUT_PULLUP);
  //pinMode(3, INPUT_PULLUP);  // havent got this switch yet


  // Initialize Joystick Library
  Joystick.begin();


  // code for load cell reader
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue;    // calibration value (see example file "Calibration.ino")
  calibrationValue = 696.0;  // uncomment this if you want to set the calibration value in the sketch
#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000;  // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;                  //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1)
      ;
  } else {
    LoadCell.setCalFactor(calibrationValue);  // set calibration value (float)
    Serial.println("Startup is complete");
  }
}





void loop() {

  // Read pin values
  for (int index = 0; index < 16; index++) {
    int currentButtonState = !digitalRead(buttonMap[index]);
    if (currentButtonState != lastButtonState[index]) {
      switch (index) {
        case 0:  // UP
          if (currentButtonState == 1) {
            Joystick.setYAxis(-1);
            Serial.println("Gear UP");
          } else {
            Joystick.setYAxis(0);
          }
          break;
        case 1:  // DOWN
          if (currentButtonState == 1) {
            Joystick.setYAxis(1);
            Serial.println("Gear DOWN");
          } else {
            Joystick.setYAxis(0);
          }
          break;
      }
      lastButtonState[index] = currentButtonState;
    }
  }

  delay(10);

  static boolean newDataReady = 0;
  const int serialPrintInterval = 0;  //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}
