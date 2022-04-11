/*
 * ButtonConfig.h - Inheriting Nexion's Library, a library for NIR's screen button configurations.
 * Created by Loo S H, April 4, 2022.
 */

#include "ButtonConfig.h"
#include <Arduino.h>
#include "ControlPanel.h"

//(GPIO)Relay selects the output voltage & LED  (USES LOW-LEVEL TRIGGER)
#define RELAY1        26
#define RELAY2        25   
#define RELAY3        33
#define RELAY4        32   //Not Used

//(GPIO)PWM selects the frequency 
#define PWM1          27
#define PWM2          14
#define PWM3          12

void b1PushCallBack(void *ptr){   //Home to CP
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.DisplayPage(1);
  Serial.println("Control Panel Page");
}

void b10PushCallBack(void *ptr){  //-Freq
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.FreqControl(0);
  Serial.println("Reducing freq");
}

void b11PushCallBack(void *ptr){  //+Freq
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.FreqControl(1);
  Serial.println("Increasing freq");
}


void b0PushCallBack(void *ptr){   //Home to Session
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.DisplayPage(2);
  Serial.println("Session Page");
}


void sw0PopCallBack(void *ptr){ //Read current state of all switch
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.CurrentInt(); 
  Serial.println("sw0");
}


void sw20PopCallBack(void *ptr){  //Start Session
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.StartSession();
  Serial.println("sw20");
}

void b12PushCallBack(void *ptr){  //To Home
  ControlPanel CP(RELAY1, RELAY2, RELAY3, RELAY4, PWM1, PWM2, PWM3);
  CP.DisplayPage(0);
  Serial.println("Home page");
}
