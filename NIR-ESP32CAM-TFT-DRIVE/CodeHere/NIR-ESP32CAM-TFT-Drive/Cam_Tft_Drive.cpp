#include "Cam_Tft_Drive.h"
#include <Arduino.h>

const char* Cam_Tft_Drive::ssid = "CebelleLaptop";  // Change to own SSID
const char* Cam_Tft_Drive::password = "123456789"; // Change to own WiFi password

const char* Cam_Tft_Drive::ntpServer = "pool.ntp.org";
const long  Cam_TftDrive::gmtOffset_sec = 28800;
const int   Cam_Tft_Drive::daylightOffset_sec = 1;

Cam_Tft_Drive::Cam_Tft_Drive(){
  
}

void Cam_Tft_Drive::buttonEvent(const int buttonPin){
  int reading = digitalRead(buttonPin);
  if (reading != _lastButtonState) {
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    if (reading != _buttonState) {
      _buttonState = reading;
      
      if (_buttonState == HIGH) {
        EEPROM.write(1, 1);
        EEPROM.commit();
        ESP.restart();
      }
    }
  }
  _lastButtonState = reading;
}

String Cam_Tft_Drive::checkDir(){
  fs::FS &fs = SD_MMC; 
  String dirPath = "/" + String(currDate());
  if(fs.exists(dirPath.c_str())){
    Serial.println("File for today exists");
  } else {
    Serial.println("Creating file for today");
    fs.mkdir(dirPath.c_str());
    EEPROM.write(0,0);                                             //reset pictureNumber to 0 everyday
    EEPROM.commit();
  }
  return dirPath;
}


String Cam_Tft_Drive::currDate(){
  //Connect to WiFi for local time callibration
  WiFi.begin(CTD.ssid, CTD.password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  configTime(CTD.gmtOffset_sec, CTD.daylightOffset_sec, CTD.ntpServer);
  printLocalTime();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
    
  struct tm timeinfo;
  char dd[3];
  char mm[11];
  char yy[5];
  String ddMMyy;
  if (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  strftime(dd,3, "%d", &timeinfo);
  strftime(mm,11, "%B", &timeinfo);
  strftime(yy,5, "%Y", &timeinfo);
  ddMMyy = String(dd) + "-" + String(mm) + "-" + String(yy);
  return ddMMyy;
}


void Cam_Tft_Drive::printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
