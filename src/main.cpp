#include <Arduino.h>
#include <stdio.h>
#include <Adafruit_SSD1306.h>
#include "Circular_Gauge.h"
#include "LC1.h"

// [x] get oled working
// [x] read data from serial
// [x] display on oled
// [ ] send over nrf

LC1 lc1;
Circular_Gauge gauge;

int stateId = STATE_UNKNOWN;

void dataChanged(int data){
  if (data <= 1)
    Serial.print("Packet ");
  
  Serial.print(data);

  if (data == 6)
    Serial.println();
}

void stateIdChanged(int newStateId)
{
  Serial.print("State changed from ");
  Serial.print(stateId);
  Serial.print(" to ");
  Serial.println(newStateId);
 
  stateId = newStateId;

  switch (stateId) {
    case STATE_NORMAL:
      Serial.println("AFR Mode");
      gauge.setupGauge(GAUGE_TYPE_ARC, AFR_MIN, AFR_MAX, "AFR");
      break;
    case STATE_O2:
      Serial.println("O2 Mode");
      gauge.setupGauge(GAUGE_TYPE_BAR, 0, 21, "O2 %");
      break;
    case STATE_FREE_AIR_CALIBRATION_IN_PROGRESS:
      Serial.println("Air Cal Mode");
      gauge.setupGauge(GAUGE_TYPE_TXT, 0, 0, "AIR CAL IN");
      gauge.setGaugeValue("PROGRESS");
      break;
    case STATE_NEEDS_FREE_AIR_CALIBRATION:
      Serial.println("Air Cal Needed");
      gauge.setupGauge(GAUGE_TYPE_TXT, 0, 0, "AIR CAL");
      gauge.setGaugeValue("NEEDED");
      break;
    case STATE_WARMUP:
      Serial.println("Warmup Mode");
      gauge.setupGauge(GAUGE_TYPE_BAR, 0, 100, "WARM UP");
      break;
    case STATE_HEATER_CALIBRATION:
      Serial.println("Heat Cal Mode");
      gauge.setupGauge(GAUGE_TYPE_BAR, 10, 0, "HEAT CAL");
      break;
    case STATE_ERROR:
      Serial.println("Error Mode");
      gauge.setupGauge(GAUGE_TYPE_TXT, 0, 0, "ERROR");
    break;
  }
}

void afrChanged(float afr, float lambda) {
  gauge.setGaugeValue(afr);
}

void o2Changed(float o2) {
  gauge.setGaugeValue(o2);
}

void warmupChanged(float warmup) {
  gauge.setGaugeValue(warmup);
}

void calibrationCountChanged(int calibrationCountdown) {
  gauge.setGaugeValue(calibrationCountdown);
}

void errorChanged(int error) 
{
  const char* msg;
  switch (error)
  {
    case ERROR_HEATER_SHORT: msg = "Heat Short"; break;
    case ERROR_HEATER_OPEN: msg = "Heat Open"; break;
    case ERROR_PUMP_SHORT: msg = "Pump Short"; break;
    case ERROR_PUMP_OPEN: msg = "Pump Open"; break;
    case ERROR_REF_OPEN: msg = "Ref Open"; break;
    case ERROR_REF_SHORT: msg = "Ref Short"; break;
    case ERROR_SOFTWARE_ERROR: msg = "Software"; break;
    case ERROR_SENSOR_TIMING: msg = "Sensor"; break;
    case ERROR_SUPPLY_LOW: msg = "Supply Low"; break;
  }

  gauge.setGaugeValue(msg);
}

void splash() {
  gauge.print(22, 17, 5, INVERSE, "LC1");
}

void setup()   {             
  Serial.begin(19200); //USB Serial
  Serial1.begin(19200);

  lc1.setDataCallback(&dataChanged);
  lc1.setStateIdCallback(&stateIdChanged);
  lc1.setAFRCallback(&afrChanged);
  lc1.setO2Callback(&o2Changed);
  lc1.setWarmupCallback(&warmupChanged);
  lc1.setErrorCallback(&errorChanged);
  lc1.setCalibrationCountdownCallback(&calibrationCountChanged);

  gauge.begin();
  splash();
}

void loop() {
  if (Serial1.available())
  {
    lc1.processData(Serial1.read());
  }
}