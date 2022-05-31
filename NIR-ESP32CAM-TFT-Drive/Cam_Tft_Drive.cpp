#include "Cam_Tft_Drive.h"
#include <Arduino.h>

Cam_Tft_Drive::Cam_Tft_Drive(){
  _ssid = "CebelleLaptop";
  _password = "123456789";
}

camera_fb_t* capture(){
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  return fb;
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
    EEPROM.write(0,0);                               //reset pictureNumber to 0 everyday
    EEPROM.commit();
  }
  return dirPath;
}


String Cam_Tft_Drive::currDate(){
  //Connect to WiFi for local time callibration
  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = 28800;
  const int daylightOffset_sec = 1;
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
    
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


void Cam_Tft_Drive::showingImage(){
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


void Cam_Tft_Drive::saveToSD(){
  
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
  }
  
  camera_fb_t *fb = capture();   
  
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
    
  }else{
    String path = String(checkDir())+ "/picture" + String(_pictureNumber) +".jpg";
    _pictureNumber = EEPROM.read(0) + 1;
    fs::FS &fs = SD_MMC; 
    Serial.printf("Picture file name: %s\n", path.c_str());
    File file = fs.open(path.c_str(), FILE_WRITE);
    
    if(!file){
      Serial.println("Failed to open file in writing mode");
    } 
    else {
      file.write(fb->buf, fb->len);                               // payload (image), payload length
      Serial.printf("Saved file to path: %s\n", path.c_str());
      EEPROM.write(0, _pictureNumber);                             //update picture number
      EEPROM.commit();
    }
    file.close();
    esp_camera_fb_return(fb); 
  }
  EEPROM.write(1,0);                                              //set to savetoSD mode
  EEPROM.commit();
  ESP.restart();
}

//---------------Send to Drive functions-------------//

String Cam_Tft_Drive::sendCapturedImage() {
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    WiFi.begin(_ssid, _password); 
    delay(500);
    Serial.print(".");
  } 
  
  String myScript = "/macros/s/AKfycbwauOMgIuxKscX76fV2I9kZ9AmVtSJNJpTLV9I0Or86V_TBXh3R1jDgXfDXkWYhhnru/exec";
  String myLineNotifyToken = "myToken=**********";    //Line Notify Token. You can set the value of xxxxxxxxxx empty if you don't want to send picture to Linenotify.
  String myFoldername = "&myFoldername=NIRCameraImages";
  String myFilename = "&myFilename=ESP-CAM.jpg";
  String myImage = "&myFile=";
  const char* myDomain = "script.google.com";
  String getAll="", getBody = "";
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get(); 
   
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = myLineNotifyToken+myFoldername+myFilename+myImage;
    
    client_tcp.println("POST " + myScript + " HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(Data.length()+imageFile.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Connection: keep-alive");
    client_tcp.println();
    
    client_tcp.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1000) {
      client_tcp.print(imageFile.substring(Index, Index+1000));
    }
    esp_camera_fb_return(fb);
    
    int waitTime = 100000;   // timeout 10 seconds
    long startTime = millis();
    boolean state = false;
    
    while ((startTime + waitTime) > millis())
    {
      Serial.print(",");
      delay(100);    
      
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);        
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  
  return getBody;
}

String Cam_Tft_Drive::urlencode(String str){
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
