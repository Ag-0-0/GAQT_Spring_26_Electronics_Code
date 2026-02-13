#include <Wire.h>
#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <SparkFun_SCD4x_Arduino_Library.h>
#include "DFRobot_OzoneSensor.h"

SCD4x ozoneSensor;
SensirionI2CSen5x sen5x;

struct Reading {
  float value;
  float upperbound;
  float lowerbound;
  bool isactive;
};

const int number_of_readings = 11;
Reading readings[number_of_readings];

void setup_sen55_sensor(int in1, int in2) {
  sen5x.begin(Wire);
  uint16_t error = sen5x.deviceReset();
  char errorMessage[256];
    if (error) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    
   // Set temperature offset for the sensor
    float tempOffset = 0.0;
    error = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (error) {
        //Serial.print("Error trying to execute setTemperatureOffsetSimple(): ");
        errorToString(error, errorMessage, 256);
        //Serial.println(errorMessage);
    } else {
        Serial.print("Temperature Offset set to ");
        Serial.print(tempOffset);
        Serial.println(" deg. Celsius (SEN54/SEN55 only)");
        pinMode(in1, OUTPUT); //OUTPUT means its configured to act as a digital (on / off) output
        pinMode(in2, OUTPUT);
    }

  // Start Measurement
  error = sen5x.startMeasurement();
}

void setup_ozone_sensor() {
  ozoneSensor.enableDebugging(); //Gives helpful debugging messages
  if(ozoneSensor.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring / address. Freezing..."));
    exit(-1);
  }
}

void setup_fan(int fan_pin) {
  pinMode(fan_pin, OUTPUT);
}

void setup_readings() { // TODO
  readings[0].upperbound = 1000000;  // Mass Concentration Pm1p0
  readings[0].lowerbound = 1000;  // Mass Concentration Pm1p0
  readings[1].upperbound = 1000000;  // Mass Concentration Pm2p5
  readings[1].lowerbound = 1000;  // Mass Concentration Pm2p5
  readings[2].upperbound = 1000000;  // Mass Concentration Pm4p0
  readings[2].lowerbound = 1000;  // Mass Concentration Pm4p0
  readings[3].upperbound = 1000000;  // Mass Concentration Pm10p0
  readings[3].lowerbound = 1000;  // Mass Concentration Pm10p0
  readings[4].upperbound = 1000000;  // Ambient Humidity
  readings[4].lowerbound = 1000;  // Ambient Humidity
  readings[5].upperbound = 1000000;  // Ambient Temperature
  readings[5].lowerbound = 1000;  // Ambient Temperature
  readings[6].upperbound = 1000000;  // VOC Index
  readings[6].lowerbound = 1000;  // VOC Index
  readings[7].upperbound = 1000000;  // nox Index
  readings[7].lowerbound = 1000;  // nox Index
  readings[8].upperbound = 1000000;  // Carbon Dioxide
  readings[8].lowerbound = 1000;  // Carbon Dioxide
  readings[9].upperbound = 24;  // Temperature
  readings[9].lowerbound = 22;  // Temperature
  readings[10].upperbound = 1000000; // Humidity
  readings[10].lowerbound = 1000; // Humidity
}

void setup_bools() {
  for (int i = 0; i < number_of_readings; ++i) {
  readings[i].isactive = false;
  }
}

void setup() {
  Wire.begin();
  Serial.begin(9600); 
  setup_bools();
  setup_sen55_sensor(2, 3);
  setup_ozone_sensor();
  setup_fan(10);
  setup_readings();
}

void read_sen55() {
 uint16_t error = sen5x.readMeasuredValues(
    readings[0].value, // Mass Concentration Pm1p0
    readings[1].value, // Mass Concentration Pm2p5
    readings[2].value, // Mass Concentration Pm4p0
    readings[3].value, // Mass Concentration Pm10p0
    readings[4].value, // Ambient Humidity
    readings[5].value, // Ambient Temperature
    readings[6].value, // VOC Index
    readings[7].value  // nox Index
  );
}

void read_ozone() {
  if (!ozoneSensor.readMeasurement()) { return; }
  readings[8].value  = ozoneSensor.getCO2();
  readings[9].value  = ozoneSensor.getTemperature();
  readings[10].value = ozoneSensor.getHumidity();
}

void update_isactives() {
  bool active = false;
  float current;
  for (int i = 0; i < number_of_readings; ++i) {
    active = readings[i].isactive;
    current = readings[i].value;
    if (active){
      if (current < readings[i].lowerbound){
        active = false;
      }
    }
    else {
      if (current > readings[i].upperbound){
        active = true;
      }
    }
    readings[i].isactive = active;
  }
}

//bool should_activate_fan() {
void fan_control(float FAN) {  
  bool result = false;
  for (int i = 0; i < number_of_readings; ++i) {
  // TODO
  result = result || readings[i].isactive;
  }
  //return result;
  if (result){
    digitalWrite(FAN, HIGH);
  }
  else {
    digitalWrite(FAN, LOW);
  }
}

void loop() {
  read_sen55();
  read_ozone();
  update_isactives(); // TODO
  fan_control(10);
  //quality_descison();
  Serial.println(readings[9].value);
}
