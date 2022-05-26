#include <Nextion.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_VL6180X.h>
#include <MFRC522.h>
#include <SPI.h>
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

TaskHandle_t DualCore;

//page, id, objName (refer to Nextion Editor)
//Home Buttons (page 0)
NexButton b0 = NexButton(0,3,"b0");   //Home to Session
NexButton b1 = NexButton(0,4,"b1");   //Home to Temperature Graph

//Session Buttons (page 2)
NexButton b20 = NexButton(2,5,"b20"); //Session to Home
NexDSButton sw20 = NexDSButton(2,3,"sw0"); //Start Session

//Temperature Buttons (page 5)
NexButton b50 = NexButton(5,5,"b50");

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
  &b0,&b1,                            //Home page
  &b20,&sw20,                         //Session page
  &b50,                               //Temperature Graph page
  NULL
};

void SecondCore(void * parameter){
  while(1){
    CP.DisplayProgress();
    CP.LEDIndex();
    //vTaskDelay(1);        //Add if keep crashing
  }
}


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  pinMode(DRIVER1, OUTPUT);
  pinMode(DRIVER2, OUTPUT);
  pinMode(DRIVER3, OUTPUT);
  
  CP.LEDOff();  //Turn off all DRIVERs
  
  CP.DisplayPage(3);
  delay(3000);
  CP.DisplayPage(4);
  b0.attachPush(b0PushCallBack,&b0);
  b1.attachPush(b1PushCallBack,&b1);
  b20.attachPush(b20PushCallBack,&b20);
  sw20.attachPush(sw20PopCallBack,&sw20);
  b50.attachPush(b20PushCallBack,&b50);
  
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

  xTaskCreatePinnedToCore(SecondCore, "DualCore", 10000, NULL, 1, &DualCore, 0);
}

void loop() {
  while(loggedIn == false){   //Waits for authentication (login)
    numberID = Login.ReadRFID();
    if (Login.UserCard(numberID) == true){
      loggedIn = true;
      CP.DisplayPage(0);
    } else {
      loggedIn = false;
    }
  }
  nexLoop(nex_listen_list);             //Listens for button events

  temp = mlx.readObjectTempC();
  CP.UpdateTemp(temp);
  
  if (CP.CurrentPage() == 0){           //Home Page
    range = vl.readRange();
    CP.DisplayDist(range);
    counter = 0;
    
  } else if (CP.CurrentPage() == 2){      //Session Page
    if (counter == 0){  
      CP.PrevSelection();
      counter = 2;
    }
    /*  Shift to dual core process, to be confirmed.
    CP.DisplayProgress();
    CP.LEDIndex();*/
    
  } else if (CP.CurrentPage() == 5) {   //Temperature Graph Page
    CP.DisplayTemp(temp);
    CP.PlotTemp(counter);
    counter = 1;
  }
  delay(5);
}
