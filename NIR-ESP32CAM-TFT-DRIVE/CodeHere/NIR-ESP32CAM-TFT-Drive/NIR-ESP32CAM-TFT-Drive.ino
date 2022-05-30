/*
 *  Adapted from Vivian Ng's ESP32CAM_TFT &
 *  Rui Santo's tutorial "ESP32-CAM Take Photo and Save to MicroSD Card"
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
int pictureNumber = 0;

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
  
  TJpgDec.setJpgScale(1);           
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  
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
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  EEPROM.begin(EEPROM_SIZE);

  if (EEPROM.read(1) == 0){             //Stream camera

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

}


camera_fb_t* capture(){
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  return fb;
}

void showingImage(){
  camera_fb_t *fb = capture();
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }else{
    TJpgDec.drawJpg(0,0,(const uint8_t*)fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
}

void saveToSD(){
  
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
  }
  
  camera_fb_t *fb = capture();   
  
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
    
  }else{
    pictureNumber = EEPROM.read(0) + 1;
    String path = String(CTD.checkDir())+ "/picture" + String(pictureNumber) +".jpg";

    fs::FS &fs = SD_MMC; 
    Serial.printf("Picture file name: %s\n", path.c_str());
    File file = fs.open(path.c_str(), FILE_WRITE);
    
    if(!file){
      Serial.println("Failed to open file in writing mode");
    } 
    else {
      file.write(fb->buf, fb->len);                               // payload (image), payload length
      Serial.printf("Saved file to path: %s\n", path.c_str());
      EEPROM.write(0, pictureNumber);                             //update picture number
      EEPROM.commit();
    }
    file.close();
    esp_camera_fb_return(fb); 
  }
  EEPROM.write(1,0);                                              //set to savetoSD mode
  EEPROM.commit();
  ESP.restart();
}


void loop() {
  if (EEPROM.read(1) == 0){
    CTD.buttonEvent(buttonPin);
    showingImage();
  } else if (EEPROM.read(1) == 1){
    saveToSD();
  }
  
}
