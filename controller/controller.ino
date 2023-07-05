#include<Wire.h>
#include <TimeLib.h>
#include <AccelStepper.h>
#include <EEPROM.h>

// Data from python file
int start_day, start_month, start_year, start_hour, start_minute, start_second;
int time_zone;
double latitude, longtitude, tilt_angle;

// Motor
const int motor_step_ratio = 2038;
AccelStepper stepper(AccelStepper::FULL4WIRE, 19, 18, 15, 17);
const int angle_address = 0;
double current_angle = 0.0;
const double moving_interval = 10;
double rotation_angle = 0.0;
double motor_step = 0.0;

// LED
const int LED_pin = 13; //TODO: tbd

 
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
  EEPROM.get(angle_address, current_angle);
  double current_motor_pos = current_angle * motor_step_ratio;
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  stepper.setCurrentPosition(current_motor_pos);
  
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
      Serial.print("Current time: ");
      Serial.println(start_time);

      start_month = start_time.substring(0, 2).toInt();
      start_day = start_time.substring(3, 5).toInt();
      start_year = start_time.substring(6, 10).toInt();
      start_hour = start_time.substring(11, 13).toInt();
      start_minute = start_time.substring(14, 16).toInt();
      start_second = start_time.substring(17, 19).toInt();

      setTime(start_hour, start_minute, start_second, start_day, start_month, start_year);

      //TODO: delete section after testing
      Serial.print("Month: ");
      Serial.println(start_month);
      Serial.print("Day: ");
      Serial.println(start_day);
      Serial.print("Year: ");
      Serial.println(start_year);
      Serial.print("Hour: ");
      Serial.println(start_hour);
      Serial.print("Minute: ");
      Serial.println(start_minute);
      Serial.print("Second: ");
      Serial.println(start_second);
      Serial.print("Timezone: ");
      Serial.println(time_zone);
      Serial.print("Latitude: ");
      Serial.println(latitude);
      Serial.print("Longtitude: ");
      Serial.println(longtitude);
      Serial.print("Tilt angle: ");
      Serial.println(tilt_angle);
    }  
  }
}

void loop(){
  time_t current_time = now();

  int current_hour = hour(current_time);
  int current_minute = minute(current_time);
  int current_second = second(current_time);
  
  Serial.println("");
  Serial.print(current_hour);
  Serial.print(":");
  Serial.print(current_minute);
  Serial.print(":");
  Serial.print(current_second);

  rotational_angle = 180.5;
  double motor_step = rotational_angle * motor_step_ratio;

  current_angle = rotational_angle;
  EEPROM.put(angle_address, current_angle);
  EEPROM.end();
  
  if (stepper.distanceToGo() == 0) {
    stepper.stop();
  }
  else {
    stepper.run();
    current_angle = stepper.currentPosition() / motor_step_ratio;
    EEPROM.put(angle_address, current_angle);
    EEPROM.end();
  }

  // 6am to 8pm
  if (current_hour >= 6 && current_hour < 20) { //TODO: might need to take into account the time zone
    if (current_minute % moving_interval == 0) { //TODO: might need other method if the interval cannot be 60 divided by the interval is not an integer
      // get the rotation angle and move the motor
      motor_step = rotation_angle * motor_step_ratio;
      stepper.moveTo(motor_step);
      
    }
  }

  // 8pm to 6am
  if (current_hour >= 20 || current_hour < 6) {
    digitalWrite(LED_pin, HIGH);
  }
  else{
    digitalWrite(LED_pin, LOW);
  }

  //TODO: current and voltage regulator
}
