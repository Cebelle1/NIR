/*
 * This version of the code is used for Intelligent Series Display, with sequencing NIR leds and OTA function.
 * 
 * In order  for OTA to work, WiFi must be connected.
 */
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
#include "WebServer.h"
#include "ControlPanel.h"
#include "ButtonConfig.h"
#include "UserLogin.h"

//(GPIO)Driver selects the output voltage & LED 
#define DRIVER1        33
#define DRIVER2        26   
#define DRIVER3        14

//SPI Pins for RFID
#define RST_PIN       22
#define SS_PIN        5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

//Configs for WiFi and OTA
IPAddress local_IP(192, 168, 137, 200);   
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
WebServer server(80);
const char* host = "esp32";
const char * ssid = "CebelleLaptop";
const char * password = "123456789";


//page, id, objName (refer to Nextion Editor)
NexButton b0 = NexButton(0,3,"b0");   //Home to Session

NexButton b20 = NexButton(2,5,"b20"); //Session to Home
NexDSButton sw20 = NexDSButton(2,3,"sw0"); //Start Session

int counter = 0;  //for setting up page

double temp;
uint8_t range;

bool loggedIn = false;
int numberID = 0;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();        //IR Temp Sensor
double new_emissivity = 0.452381;                   //Refer to datasheet for formula

Adafruit_VL6180X vl = Adafruit_VL6180X(0x29);       //Time of Flight (ToF) Distance sensor
ControlPanel CP(DRIVER1, DRIVER2, DRIVER3);
UserLogin Login(SS_PIN,RST_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);


NexTouch *nex_listen_list[] = {
  &b0,                                //Home page
  &b20,&sw20,                         //Session page
  NULL
};


//OTA Webserver
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
  pinMode(DRIVER1, OUTPUT);
  pinMode(DRIVER2, OUTPUT);
  pinMode(DRIVER3, OUTPUT);
  
  CP.LEDOff();  //Turn off all DRIVERs
  
  Serial.begin(115200);
  Serial2.begin(9600);
  
  CP.DisplayPage(3);
  delay(3000);
  CP.DisplayPage(4);
  
  b0.attachPush(b0PushCallBack,&b0);
  b20.attachPush(b20PushCallBack,&b20);
  sw20.attachPush(sw20PopCallBack,&sw20);
  
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

  //REMOVE THIS LINE IN NORMAL CODE, LOGGIN IN IS DISABLED FOR NOW
  CP.DisplayPage(0);
}

void loop() {
  /*while(loggedIn == false){   //Waits for authentication (login)
    numberID = Login.ReadRFID();
    
    if (Login.UserCard(numberID) == true && Login.OTACard(numberID) == false){
      loggedIn = true;
      CP.DisplayPage(0);
    } else if (Login.OTACard(numberID) == true) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        Serial2.print("t0.txt=\"");
        Serial2.print("No WiFi");
        Serial2.print("\"");
        Serial2.write(0xFF);
        Serial2.write(0xFF);
        Serial2.write(0xFF);
        delay(100);
      }
      Serial2.print("t0.txt=\"");
      Serial2.print(String(WiFi.localIP()));
      Serial2.print("\"");
      Serial2.write(0xFF);
      Serial2.write(0xFF);
      Serial2.write(0xFF);
      StartServer();              //Set the WebServer for GUI
      while(1){
        server.handleClient();
        delay(1);
      }
    } 
  }*/
  
  nexLoop(nex_listen_list);             //Listens for button events
  
  
  if (CP.CurrentPage() == 0){           //Home Page
    //REMOVE THIS LINE IN NORMAL CODE, NO SENSORS ATTACHED WILL STOP THE CODE WITHOUT ANY ERROR THROWN
    /*range = vl.readRange(); 
    CP.DisplayDist(range);
    temp = mlx.readObjectTempC();
    CP.DisplayTemp(temp);*/
    counter = 0;
    
  } else if (CP.CurrentPage() == 2){      //Session Page
    if (counter == 0){  
      CP.PrevSelection();
      counter = 2;
    }
    
    CP.DisplayProgress();
    CP.LEDIndex();
  } 
  
}
