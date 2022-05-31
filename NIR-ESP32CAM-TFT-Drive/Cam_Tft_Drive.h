#ifndef CAM_TFT_DRIVE_H
#define CAM_TFT_DRIVE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include <TJpg_Decoder.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SD_MMC.h"
#include <TimeLib.h>
#include "time.h"
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

class Cam_Tft_Drive{

  public:
    Cam_Tft_Drive();
    
    void buttonEvent(const int buttonPin);
    String checkDir();
    String currDate();
    void printLocalTime();
    void showingImage();
    void saveToSD();

    String sendCapturedImage();
    String urlencode(String str);
    
  private:
    const char* _ssid;
    const char* _password;
    
    int _buttonState;             
    int _lastButtonState = LOW;   
    unsigned long _lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long _debounceDelay = 50;    // the debounce time; increase if the output flickers
    int _pictureNumber;
};

#endif
