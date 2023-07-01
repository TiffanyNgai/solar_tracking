#include<Wire.h>
#include <TimeLib.h> 

const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ;
int minVal = 265;
int maxVal = 402;
double y;
int start_day, start_month, start_year, start_hour, start_minute, start_second;
int time_zone;
double latitude, longtitude, tilt_angle;
const int cal_intervals[] = {0,0,0,0,0,0,0,,40};
// 10pm to 6am: no movement
// 6am to 12pm: move every 60 mins
// 12pm to 3pm: move every 20 mins
// 3pm to 6pm: move every 30 mins
// 6pm to 10pm: move every 60 mins

// 10pm to 6am: LED on, discharging
// 6am to 10pm: LED off, charging

bool send_angle = 1;
 
void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  
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
  time_t currentTime = now();

  int currentHour = hour(currentTime);
  
  int currentMinute = minute(currentTime);
  int currentSecond = second(currentTime);
  
  Serial.println("");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(":");
  Serial.print(currentSecond);

  delay(1000);
}
