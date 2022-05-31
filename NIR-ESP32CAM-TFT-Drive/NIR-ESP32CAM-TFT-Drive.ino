/*
 *  Adapted from Vivian Ng's ESP32CAM_TFT,
 *  Rui Santo's tutorial "ESP32-CAM Take Photo and Save to MicroSD Card" &
 *  Electroclinic's ESP32CAM send images to Google Drive
 *  
 *  Important Note: Please ensure WiFi connection, else program will not work. 
 *  WiFi SSID and Password can be changed in the Cam_Tft_Drive source code's constructor. 
 *  
 *  Note: When uploading this to a new ESP32-CAM, to ensure it works as programmed,
 *  please manually write 0 into the EEPROM for both register 0 and 1. 
 *  (EEPROM.write(0,0); EEPROM.write(1,0))
 */

#include <Arduino.h>
#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include <TJpg_Decoder.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SD_MMC.h"
#include <EEPROM.h>
#include <TimeLib.h>
#include "time.h"
#include "Cam_Tft_Drive.h"
#define GFXFF 1
#define FSB9 &FreeSerifBold9pt7b
#define EEPROM_SIZE 2 //Update pic no. up to 256

TFT_eSPI tft = TFT_eSPI();

const unsigned long timeout = 30000; // 30 seconds
const int buttonPin = 4;    // the number of the pushbutton pin

Cam_Tft_Drive CTD;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;    // Stop further decoding as image is running off bottom of screen
  tft.pushImage(x, y, w, h, bitmap);    //Clip image block rendering at TFT boundary
  return 1;
}


void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  
  camera_config_t config;           
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  EEPROM.begin(EEPROM_SIZE);

  if (EEPROM.read(1) == 0){             //Stream camera
      
    TJpgDec.setJpgScale(1);           
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);
    //Init display
    tft.begin();
    tft.setRotation(1);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(FSB9);

    //Init camera at display resolution 
    config.frame_size = FRAMESIZE_QQVGA; // 120 x 160
    config.jpeg_quality = 10;
    config.fb_count = 2;
    
  } else if (EEPROM.read(1) == 1){    //Take photo and save to sd
    //Init camera at bigger resolution
    
    config.frame_size = FRAMESIZE_SVGA;    //800 x 600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

}

void loop() {
  if (EEPROM.read(1) == 0){
    CTD.buttonEvent(buttonPin);
    CTD.showingImage();
  } else if (EEPROM.read(1) == 1){    
    CTD.sendCapturedImage();          //(Do not switch the order) Send image first
    CTD.saveToSD();                   //Then saveToSD, which will reset CAM to streaming mode
  }
  
}
