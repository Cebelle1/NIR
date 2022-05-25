#include <Nextion.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_VL6180X.h>
#include <MFRC522.h>
#include <SPI.h>
#include "UserLogin.h"

#define DRIVER1 33
#define DRIVER2 26
#define DRIVER3 14

#define RST_PIN       22
#define SS_PIN        5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

NexDSButton bt0 = NexDSButton(0,1,"bt0");
NexDSButton bt1 = NexDSButton(0,2,"bt1");
NexDSButton bt2 = NexDSButton(0,3,"bt2");
NexDSButton bt3 = NexDSButton(0,4,"bt3");

double temp;
uint8_t range;

bool loggedIn = false;
int numberID = 0;

String power;
bool start = false;
bool low = false;
bool medium = false;
bool high = false;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();        //IR Temp Sensor
double new_emissivity = 0.452381;

uint32_t number;
Adafruit_VL6180X vl = Adafruit_VL6180X(0x29);       //Time of Flight (ToF) Distance sensor
UserLogin Login(SS_PIN,RST_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);

NexTouch *nex_listen_list[] = {
  &bt0, &bt1, &bt2, &bt3,                  //Session page
  NULL
};

void bt0PushCallBack(void *ptr){
  bt0.getValue(&number);
  if (int(number) == 1){
    power = "low";
  }
}

void lowMode(){
  /*digitalWrite(DRIVER1, HIGH);
  delayMicroseconds(3330);
  digitalWrite(DRIVER1,LOW);
  digitalWrite(DRIVER2,HIGH);
  delayMicroseconds(3330);
  digitalWrite(DRIVER2,LOW);
  digitalWrite(DRIVER3,HIGH);
  delayMicroseconds(3330);
  DriverOff();
  delay(30);*/
 
  digitalWrite(DRIVER1, LOW);
  delayMicroseconds(3330);
  digitalWrite(DRIVER1,HIGH);
  digitalWrite(DRIVER2,LOW);
  delayMicroseconds(3330);
  digitalWrite(DRIVER2,HIGH);
  digitalWrite(DRIVER3,LOW);
  delayMicroseconds(3330);
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
  /*digitalWrite(DRIVER1, HIGH);
  delayMicroseconds(4500);
  digitalWrite(DRIVER1,LOW);
  digitalWrite(DRIVER2,HIGH);
  delayMicroseconds(4500);
  digitalWrite(DRIVER2,LOW);
  digitalWrite(DRIVER3,HIGH);
  delayMicroseconds(4500);
  DriverOff();
  delayMicroseconds(26500);*/
  
  digitalWrite(DRIVER1, LOW);
  delayMicroseconds(4500);
  digitalWrite(DRIVER1,HIGH);
  digitalWrite(DRIVER2,LOW);
  delayMicroseconds(4500);
  digitalWrite(DRIVER2,HIGH);
  digitalWrite(DRIVER3,LOW);
  delayMicroseconds(4500);
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
  /*digitalWrite(DRIVER1, HIGH);
  delayMicroseconds(6670);
  digitalWrite(DRIVER1,LOW);
  digitalWrite(DRIVER2,HIGH);
  delayMicroseconds(6670);
  digitalWrite(DRIVER2,LOW);
  digitalWrite(DRIVER3,HIGH);
  delayMicroseconds(6670);
  DriverOff();
  delay(20);*/
  
  digitalWrite(DRIVER1, LOW);
  delayMicroseconds(6670);
  digitalWrite(DRIVER1,HIGH);
  digitalWrite(DRIVER2,LOW);
  delayMicroseconds(6670);
  digitalWrite(DRIVER2,HIGH);
  digitalWrite(DRIVER3,LOW);
  delayMicroseconds(6670);
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

void DriverOff(){
  /*digitalWrite(DRIVER1,LOW);
  digitalWrite(DRIVER2,LOW);
  digitalWrite(DRIVER3,LOW);*/
  
  digitalWrite(DRIVER1,HIGH);
  digitalWrite(DRIVER2,HIGH);
  digitalWrite(DRIVER3,HIGH);
}

void DriverOn(){
  /*digitalWrite(DRIVER1,HIGH);
  digitalWrite(DRIVER2,HIGH);
  digitalWrite(DRIVER3,HIGH);*/

  digitalWrite(DRIVER1,LOW);
  digitalWrite(DRIVER2,LOW);
  digitalWrite(DRIVER3,LOW);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600);

  pinMode(DRIVER1, OUTPUT);
  pinMode(DRIVER2, OUTPUT);
  pinMode(DRIVER3, OUTPUT);

  bt0.attachPush(bt0PushCallBack, &bt0);
  bt1.attachPush(bt1PushCallBack, &bt1);
  bt2.attachPush(bt2PushCallBack, &bt2);
  bt3.attachPush(bt3PushCallBack, &bt3);

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
    Serial.println("Driver OFF");
  }
  
}
