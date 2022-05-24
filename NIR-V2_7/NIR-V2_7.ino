#include <Nextion.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_VL6180X.h>
#include <MFRC522.h>
#include <SPI.h>
#include "ControlPanel.h"
#include "ButtonConfig.h"
#include "UserLogin.h"

//(GPIO)Relay selects the output voltage & LED  (USES LOW-LEVEL TRIGGER)
#define RELAY1        26
#define RELAY2        25   
#define RELAY3        33
#define RELAY4        32   //Not Used

//(GPIO)PWM selects the frequency 
#define PWM1          27
#define PWM2          14
#define PWM3          12

//SPI Pins for RFID
#define RST_PIN       22
#define SS_PIN        5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

TaskHandle_t LEDPWM;

//page, id, objName (refer to Nextion Editor)
//Home Buttons (page 0)
NexButton b1 = NexButton(0,6,"b1");   //Home to CP
NexButton b0 = NexButton(0,5,"b0");   //Home to Session

//CP Buttons (page 1)
NexButton b10 = NexButton(1,3, "b10"); //-Freq
NexButton b11 = NexButton(1,4, "b11"); //+Freq
NexButton b12 = NexButton(1,9, "b12"); //CP to Home
NexText FreqBox = NexText(1,1, "FreqBox");
NexDSButton sw0 = NexDSButton(1,6,"sw0"); //625nm LED  
NexDSButton sw1 = NexDSButton(1,7,"sw1"); //850nm LED
NexDSButton sw2 = NexDSButton(1,8,"sw2"); //940nm LED

//Session Buttons (page 2)
NexButton b20 = NexButton(2,5,"b20"); //Session to Home
NexDSButton sw20 = NexDSButton(2,3,"sw0"); //Start Session

char *FREQUENCY = "60"; 

int counter = 0;  //for setting up page

double temp;
uint8_t range;

bool loggedIn = false;
int numberID = 0;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();        //IR Temp Sensor
double new_emissivity = 0.452381;                   //Refer to datasheet for formula

Adafruit_VL6180X vl = Adafruit_VL6180X(0x29);       //Time of Flight (ToF) Distance sensor
ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
UserLogin Login(SS_PIN,RST_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);


NexTouch *nex_listen_list[] = {
  &b0,&b1,                        //Home page
  &b10,&b11,&b12,&sw0,&sw1,&sw2,  //Control panel page
  &b20,&sw20,                     //Session page
  NULL
};


void SecondCore(void * parameter){
  for (;;){
    CP.LEDFreq();
    vTaskDelay(1);
  }
}


void setup() {
  
  Serial.begin(115200);
  Serial2.begin(9600);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(PWM3, OUTPUT);
  
  CP.LEDOff();  //Turn off all relays
  
  FreqBox.setText(FREQUENCY);
  CP.DisplayPage(3);
  delay(3000);
  CP.DisplayPage(4);
  b0.attachPush(b0PushCallBack,&b0);
  b1.attachPush(b1PushCallBack,&b1);
  b10.attachPush(b10PushCallBack,&b10);
  b11.attachPush(b11PushCallBack,&b11);
  b12.attachPush(b12PushCallBack,&b12);
  sw0.attachPush(sw0PopCallBack,&sw0);
  sw1.attachPush(sw0PopCallBack,&sw1);
  sw2.attachPush(sw0PopCallBack,&sw2);
  b20.attachPush(b12PushCallBack,&b20);
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
  xTaskCreatePinnedToCore(SecondCore, "LEDPWM", 10000, NULL, 1, &LEDPWM, 0);
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

  nexLoop(nex_listen_list);   //Listens for button events
  
  if (CP.CurrentPage() == 0){
    temp = mlx.readObjectTempC();
    range = vl.readRange();
    CP.Display_TempDist(temp,range);
    counter = 0;
  } else if (CP.CurrentPage() == 1){
    if (counter == 0){
      CP.PrevSelection();
      counter = 1;
    }
  }
  else if (CP.CurrentPage() == 2){
    if (counter == 0){
      CP.PrevSelection();
      counter = 1;
    }
    CP.DisplayProgress();
  }
  delay(5);
}
