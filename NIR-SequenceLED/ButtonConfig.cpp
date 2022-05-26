/*
 * ButtonConfig.h - Inheriting Nexion's Library, a library for NIR's screen button configurations.
 * Created by Loo S H, May 26, 2022.
 */

#include "ButtonConfig.h"
#include <Arduino.h>
#include "ControlPanel.h"

//(GPIO)Driver selects the output voltage & LED  
#define DRIVER1        33
#define DRIVER2        26   
#define DRIVER3        14


void b1PushCallBack(void *ptr){   //Home to Temperature graph
  ControlPanel CP(DRIVER1, DRIVER2, DRIVER3);
  CP.DisplayPage(5);
  Serial.println("Temperature Graph Page");
}


void b0PushCallBack(void *ptr){   //Home to Session
  ControlPanel CP(DRIVER1, DRIVER2, DRIVER3);
  CP.DisplayPage(2);
  Serial.println("Session Page");
}

void sw20PopCallBack(void *ptr){  //Start Session
  ControlPanel CP(DRIVER1, DRIVER2, DRIVER3);
  CP.StartSession();
  Serial.println("sw20");
}

void b20PushCallBack(void *ptr){
  ControlPanel CP(DRIVER1, DRIVER2, DRIVER3);
  CP.DisplayPage(0);
  Serial.println("Home page");
}
