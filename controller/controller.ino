#include<Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ;
int minVal = 265;
int maxVal = 402;
double y;
int day, month, year, hour, minute, second;
int time_zone;
double latitude, longtitude;

bool send_angle = 1;
 
void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);

  rtc.begin();
  
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
      Serial.print("Current time: ");
      Serial.println(start_time);

      month = start_time.substring(0, 2).toInt();
      day = start_time.substring(3, 5).toInt();
      year = start_time.substring(6, 10).toInt();
      hour = start_time.substring(11, 13).toInt();
      minute = start_time.substring(14, 16).toInt();
      second = start_time.substring(17, 19).toInt();

      rtc.adjust(DateTime(year, month, day, hour, minute, second));

      //TODO: delete section after testing
      Serial.print("Month: ");
      Serial.println(month);
      Serial.print("Day: ");
      Serial.println(day);
      Serial.print("Year: ");
      Serial.println(year);
      Serial.print("Hour: ");
      Serial.println(hour);
      Serial.print("Minute: ");
      Serial.println(minute);
      Serial.print("Second: ");
      Serial.println(second);
      Serial.print("Timezone: ");
      Serial.println(time_zone);
      Serial.print("Latitude: ");
      Serial.println(latitude);
      Serial.print("Longtitude: ");
      Serial.println(longtitude);
    }
  }
}



void printDateTime(const DateTime& dt) {
  Serial.print(dt.year(), DEC);
  Serial.print('/');
  Serial.print(dt.month(), DEC);
  Serial.print('/');
  Serial.print(dt.day(), DEC);
  Serial.print(' ');
  Serial.print(dt.hour(), DEC);
  Serial.print(':');
  Serial.print(dt.minute(), DEC);
  Serial.print(':');
  Serial.print(dt.second(), DEC);
  Serial.println();
}

void loop(){
  DateTime current_time = rtc.now();
  printDateTime(current_time);
  delay(1000);
}
