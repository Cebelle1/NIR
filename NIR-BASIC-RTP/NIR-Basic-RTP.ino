//This is coded in a stupid way where if you select the wrong button u gotta restart the device.
#include <Nextion.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_VL6180X.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "UserLogin.h"
#include "WebServer.h"

#define DRIVER1 33
#define DRIVER2 26
#define DRIVER3 14

#define RST_PIN       22
#define SS_PIN        5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18


IPAddress local_IP(192, 168, 137, 200);   //New dog
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
WebServer server(80);
const char* host = "esp32";
const char * ssid = "CebelleLaptop";
const char * password = "123456789";

NexDSButton bt0 = NexDSButton(0,1,"bt0");   //Low
NexDSButton bt1 = NexDSButton(0,2,"bt1");   //Med
NexDSButton bt2 = NexDSButton(0,3,"bt2");   //High
NexDSButton bt3 = NexDSButton(0,4,"bt3");   //Start
NexDSButton bt4 = NexDSButton(0,10,"bt4");    //LED1
NexDSButton bt5 = NexDSButton(0,11,"bt5");    //LED2
NexDSButton bt6 = NexDSButton(0,12,"bt6");    //LED3
NexDSButton bt7 = NexDSButton(0,13,"bt7");    //OTA button



double temp;
uint8_t range;

bool loggedIn = false;
int numberID = 0;

String power;
bool start = false;
bool low = false;
bool medium = false;
bool high = false;
bool led1 = false;
bool led2 = false;
bool led3 = false;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();        //IR Temp Sensor
double new_emissivity = 0.452381;

uint32_t number;
Adafruit_VL6180X vl = Adafruit_VL6180X(0x29);       //Time of Flight (ToF) Distance sensor
UserLogin Login(SS_PIN,RST_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);

NexTouch *nex_listen_list[] = {
  &bt0, &bt1, &bt2, &bt3, &bt4, &bt5, &bt6, &bt7,          
  NULL
};

void bt0PushCallBack(void *ptr){
  bt0.getValue(&number);
  if (int(number) == 1){
    power = "low";
  }
}

void lowMode(){
  if (led1 == true){
    digitalWrite(DRIVER1, LOW);
    delayMicroseconds(3330);
    digitalWrite(DRIVER1,HIGH);
  }
  if (led2 == true){ 
    digitalWrite(DRIVER2,LOW);
    delayMicroseconds(3330);
    digitalWrite(DRIVER2,HIGH);
  }
  if (led3 == true){
    digitalWrite(DRIVER3,LOW);
    delayMicroseconds(3330);
  }
    DriverOff();
    delay(30);
}

void bt1PushCallBack(void *ptr){
  bt1.getValue(&number);
  if(int(number) == 1){
    power = "medium";
  }
}

void mediumMode(){
 if (led1 == true){
    digitalWrite(DRIVER1, LOW);
    delayMicroseconds(4500);
    digitalWrite(DRIVER1,HIGH);
 }
 if (led2 == true){
    digitalWrite(DRIVER2,LOW);
    delayMicroseconds(4500);
    digitalWrite(DRIVER2,HIGH);
 }
 if (led3 == true){
    digitalWrite(DRIVER3,LOW);
    delayMicroseconds(4500);
 }
    DriverOff();
    delayMicroseconds(26500);
}

void bt2PushCallBack(void *ptr){
  bt2.getValue(&number);
  if(int(number) == 1){
    power = "high";
  }
}

void highMode(){
 if (led1 == true){
    digitalWrite(DRIVER1, LOW);
    delayMicroseconds(6670);
    digitalWrite(DRIVER1,HIGH);
 }
 if (led2 == true){
    digitalWrite(DRIVER2,LOW);
    delayMicroseconds(6670);
    digitalWrite(DRIVER2,HIGH);
 }
 if (led3 == true){
    digitalWrite(DRIVER3,LOW);
    delayMicroseconds(6670);
 }
    DriverOff();
    delay(20);
}

void bt3PushCallBack(void *ptr){
  bt3.getValue(&number);
  if (int(number) == 1){
    start = true;
  } else {
    start = false;
  }
}

void bt4PushCallBack(void *ptr){
  led1 = true;
}

void bt5PushCallBack(void *ptr){
  led2 = true;
}

void bt6PushCallBack(void *ptr){
  led3 = true;
}

void DriverOff(){
  digitalWrite(DRIVER1,HIGH);
  digitalWrite(DRIVER2,HIGH);
  digitalWrite(DRIVER3,HIGH);
}

void DriverOn(){
  digitalWrite(DRIVER1,LOW);
  digitalWrite(DRIVER2,LOW);
  digitalWrite(DRIVER3,LOW);
}

void bt7PushCallBack(void *ptr){
  Serial2.print("t4.txt=\"");
  Serial2.print("OTA Mode");
  Serial2.print("\"");
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  
  StartServer();
  
  while(1){
    server.handleClient();
    delay(1);
  }
}

void StartServer(){
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600);

  WiFi.mode(WIFI_STA);
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(ssid);
    Serial.println(password);
  }

  
  pinMode(DRIVER1, OUTPUT);
  pinMode(DRIVER2, OUTPUT);
  pinMode(DRIVER3, OUTPUT);

  bt0.attachPush(bt0PushCallBack, &bt0);
  bt1.attachPush(bt1PushCallBack, &bt1);
  bt2.attachPush(bt2PushCallBack, &bt2);
  bt3.attachPush(bt3PushCallBack, &bt3);
  bt4.attachPush(bt4PushCallBack, &bt4);
  bt5.attachPush(bt5PushCallBack, &bt5);
  bt6.attachPush(bt6PushCallBack, &bt6);
  bt7.attachPush(bt7PushCallBack, &bt7);
  
  if (!mlx.begin(0x5A)) {
    Serial.println("Error connecting to MLX sensor.");
  }

  if (!vl.begin()) {
    Serial.println("Failed to connect to ToF sensor.");
  }
  mlx.writeEmissivity(new_emissivity);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  
}

void loop() {
  /*while (loggedIn == false){
    numberID = Login.ReadRFID();
    if (Login.UserCard(numberID) == true){
      loggedIn = true;
      Serial2.print("t4.txt=\"");
      Serial2.print("Logged In");
      Serial2.print("\"");
      Serial2.write(0xFF);
      Serial2.write(0xFF);
      Serial2.write(0xFF);
   
    }else {
      loggedIn = false;
    }
  }*/

  nexLoop(nex_listen_list);   //Listens for button events
  //t2 temp t3 dist
  /*Serial2.print("t2.txt=\"");
  Serial2.print(String(mlx.readObjectTempC()));
  Serial2.print("\"");
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);

  Serial2.print("t3.txt=\"");
  Serial2.print(String(vl.readRange()));
  Serial2.print("\"");
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);*/

  if(start == true){
    if (power == "low"){
      lowMode();
      Serial.println("Low Mode");
    } else if (power == "medium"){
      mediumMode();
      Serial.println("Medium Mode");
    } else if (power == "high"){
      highMode();
      Serial.println("High mode");
    }
  } else {
    DriverOff();
    //Serial.println("Driver OFF");
  }
  
}
