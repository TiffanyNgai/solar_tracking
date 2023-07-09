#include<Wire.h>
#include <TimeLib.h>
#include <AccelStepper.h>
#include <EEPROM.h>
#include <Adafruit_INA219.h>

// Data from python file
int start_day, start_month, start_year, start_hour, start_minute, start_second;
int time_zone;
double latitude, longtitude, tilt_angle;

// Motor
const double motor_step_ratio = 2048.0 / 360.0;
const int motor_interrupt_pin = 8;  // Connect to interrupt pin
volatile bool motorState = false;  // Motor state (ON/OFF)
AccelStepper stepper(AccelStepper::FULL4WIRE, 2, 4, 3, 5);
const int angle_address = 0;
double current_angle = 0.0;
unsigned long moving_interval_sec = 25 * 60;
unsigned long current_t_sec, previous_t_sec;
double rotation_angle = 0.0;
double motor_step = 0.0;
double fitted_m, fitted_b;
int daylight_start_min;


// LED
const int LED_pin = 13; //TODO: tbd

// Current sensor
Adafruit_INA219 ina219;
const int base_pin = 7;
int del = 400;
float current_mA = 0;
float voltage = 0;
float power_mW = 0;

 
void setup(){
  Serial.begin(115200);
  
  // Accelerometer
  const int MPU_addr=0x68;
  int16_t AcX,AcY,AcZ;
  int minVal = 265;
  int maxVal = 402;
  double y;
  bool send_angle = 1;
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Motor
  EEPROM.begin();
  current_angle = EEPROM.read(angle_address);
  double current_motor_pos = current_angle * motor_step_ratio;
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  stepper.setCurrentPosition(current_motor_pos);

  // Current sensor
  ina219.begin();
  pinMode(base_pin, OUTPUT);
  ina219.setCalibration_16V_400mA();
  
  
  while(send_angle){
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);
    AcX=Wire.read()<<8|Wire.read();
    AcY=Wire.read()<<8|Wire.read();
    AcZ=Wire.read()<<8|Wire.read();
    int xAng = map(AcX,minVal,maxVal,-90,90);
    int zAng = map(AcZ,minVal,maxVal,-90,90);
     
    y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
    Serial.println(y);
    delay(400);
    
    if (Serial.available()) {
      send_angle = 0;
      String start_time = Serial.readStringUntil('\n');
      time_zone = atoi(Serial.readStringUntil('\n').c_str());
      latitude = atof(Serial.readStringUntil('\n').c_str());
      longtitude = atof(Serial.readStringUntil('\n').c_str());
      tilt_angle = atof(Serial.readStringUntil('\n').c_str());
      daylight_start_min = atoi(Serial.readStringUntil('\n').c_str());
      fitted_m = atof(Serial.readStringUntil('\n').c_str());
      fitted_b = strtod(Serial.readStringUntil('\n').c_str(), NULL);

      start_month = start_time.substring(0, 2).toInt();
      start_day = start_time.substring(3, 5).toInt();
      start_year = start_time.substring(6, 10).toInt();
      start_hour = start_time.substring(11, 13).toInt();
      start_minute = start_time.substring(14, 16).toInt();
      start_second = start_time.substring(17, 19).toInt();

      setTime(start_hour, start_minute, start_second, start_day, start_month, start_year);

//      //TODO: delete section after testing
//      Serial.print("Current time: ");
//      Serial.println(start_time);
//      Serial.print("Month: ");
//      Serial.println(start_month);
//      Serial.print("Day: ");
//      Serial.println(start_day);
//      Serial.print("Year: ");
//      Serial.println(start_year);
//      Serial.print("Hour: ");
//      Serial.println(start_hour);
//      Serial.print("Minute: ");
//      Serial.println(start_minute);
//      Serial.print("Second: ");
//      Serial.println(start_second);
//      Serial.print("Timezone: ");
//      Serial.println(time_zone);
//      Serial.print("Latitude: ");
//      Serial.println(latitude);
//      Serial.print("Longtitude: ");
//      Serial.println(longtitude);
//      Serial.print("Tilt angle: ");
//      Serial.println(tilt_angle);
//      Serial.print("Daylight start min: ");
//      Serial.println(daylight_start_min);
//      Serial.print("Fitted m: ");
//      Serial.println(fitted_m);
//      Serial.print("Fitted b: ");
//      Serial.println(fitted_b);
    }  
  }

  
  time_t current_time = now();

  int current_hour = hour(current_time);
  int current_minute = minute(current_time);
  int current_second = second(current_time);

  previous_t_sec = current_hour * 3600 + current_minute * 60 + current_second;
}

void loop(){
  time_t current_time = now();

  int current_hour = hour(current_time);
  int current_minute = minute(current_time);
  int current_second = second(current_time);
  
  current_t_sec = current_hour * 3600 + current_minute * 60 + current_second;

//  rotation_angle = 180.5;
//  double motor_step = rotation_angle * motor_step_ratio;
//
//  current_angle = rotation_angle;
//  EEPROM.put(angle_address, current_angle);
//  EEPROM.end();
//  
//  if (stepper.distanceToGo() == 0) {
//    stepper.stop();
//  }
//  else {
//    stepper.run();
//    current_angle = stepper.currentPosition() / motor_step_ratio;
//    EEPROM.put(angle_address, current_angle);
//    EEPROM.end();
//  }

  //testing motor
  moving_interval_sec = 5;
  if (current_t_sec - previous_t_sec >= moving_interval_sec) {
    double x = hour() * 60 + minute() - daylight_start_min;
    rotation_angle = fitted_m * x + fitted_b;
    if (rotation_angle > 90){ 
      rotation_angle = 90;
    }
    else if (rotation_angle < -90) {
      rotation_angle = -90;
    }
    rotation_angle = 90;
    motor_step = rotation_angle * motor_step_ratio;
    current_angle = EEPROM.read(angle_address);
    Serial.print("motor_step_ratio: "); Serial.println(motor_step_ratio);
    Serial.print("current_angle: "); Serial.println(current_angle);
    Serial.print("rotation_angle: "); Serial.println(rotation_angle);
    Serial.print("motor_step: "); Serial.println(motor_step);
    stepper.moveTo(motor_step);
    while (stepper.distanceToGo() != 0) {
      stepper.run();
    }
    stepper.disableOutputs();
    current_angle = stepper.currentPosition() / motor_step_ratio;
    EEPROM.write(angle_address, current_angle);
    previous_t_sec = current_t_sec;
  }

//  Serial.println("");
//  Serial.print("Current time: ");
//  Serial.print(current_hour);
//  Serial.print(":");
//  Serial.print(current_minute);
//  Serial.print(":");
//  Serial.print(current_second);
//  double x = current_hour * 60 + current_minute - daylight_start_min;
//  rotation_angle = fitted_m * x + fitted_b;
//  Serial.println("");
//  Serial.print("Input minute: ");
//  Serial.print(x);
//  Serial.println("");
//  Serial.print("Rotational angle: ");
//  Serial.print(rotation_angle);
  

  // 6am to 8pm
  if ((current_hour >= 6 && current_hour < 20) && (current_t_sec - previous_t_sec >= moving_interval_sec)) {
    double x = hour() * 60 + minute() - daylight_start_min;
    rotation_angle = fitted_m * x + fitted_b;
    if (rotation_angle > 90){ 
      rotation_angle = 90;
    }
    else if (rotation_angle < -90) {
      rotation_angle = -90;
    }
//    motor_step = rotation_angle * motor_step_ratio;
//    stepper.moveTo(motor_step);
    previous_t_sec = current_t_sec;
  }

//  // 8pm to 6am
//  if (current_hour >= 20 || current_hour < 6) {
//    digitalWrite(LED_pin, HIGH);
//  }
//  else{
//    digitalWrite(LED_pin, LOW);
//  }

//  delay(5000);


  // Measure power
  bool base_state = LOW;
  digitalWrite(base_pin, base_state);
  delay(del);
  voltage = ina219.getBusVoltage_V();
  
  base_state = HIGH;
  digitalWrite(base_pin, base_state);
  delay(del);
  current_mA = ina219.getCurrent_mA();
  
  power_mW = (0.7*voltage)*current_mA;
  
//  Serial.print(current_hour); Serial.print(":"); 
//  Serial.print(current_minute); Serial.print(":"); 
//  Serial.print(current_second); Serial.print(", "); 
//  Serial.print(voltage); Serial.print(", "); 
//  Serial.print(current_mA); Serial.print(", ");
//  Serial.println(power_mW);
//  
}
