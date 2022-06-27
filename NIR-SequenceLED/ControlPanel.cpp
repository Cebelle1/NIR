/*
 * ControlPanel.h - Library used for NIR's Control Panel.
 * Created by Loo S H, May 26, 2022.
 */

#include <TimeLib.h>
#include "ControlPanel.h"
#include <Arduino.h>
#include <Nextion.h>

//Static variables are used to store states throughout the runtime
int ControlPanel::_pg = 0;                        //Stores the current page
int ControlPanel::_sessionTime = 0;               //Stores the remaining time to run for the session
int ControlPanel::_sessionInt = 0;                //Stores the intensity of the session (LOW/MED/HIGH)
int ControlPanel::_startTime = 0;                 //Stores the time when session starts
int ControlPanel::_oldSessionState = 0;           //Stores the previous state of the start button
int ControlPanel::_sessionState = 0;              //Stores the state of the start button (ON/OFF) 
bool ControlPanel::_onGoingSession = false;       //Stores the state of the current session (PAUSED/STOPPED)
double ControlPanel::_pausedMinute = 0;           //Stores the amount of time past since pause 


ControlPanel::ControlPanel(int DRIVER1, int DRIVER2, int DRIVER3){
  _DRIVER1 = DRIVER1;    //625
  _DRIVER2 = DRIVER2;   //850
  _DRIVER3 = DRIVER3;   //940
}


void ControlPanel::DisplayPage(int pg){                 //Change page
  _pg = pg;
  for (int i=0; i<2; i++){
    String _stringPageS = _stringPage + pg;
    Serial2.print(_stringPageS);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
  }
}


void ControlPanel::DisplayDist(uint8_t range){    //Display distance
  _range = (range != 255)? String(range) : "";
  
  Serial2.print("DistBox.txt=\"");  
  Serial2.print(_range);  
  Serial2.print("\"");
  Serial2.write(0xFF);  
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}


void ControlPanel::DisplayTemp(double temp){    //Display Temperature Graph and Valuebox
  _degC = (temp > 0)? String(temp) : "Sensor Error";
  
  Serial2.print("TempBox.txt=\"");   //Use Serial2.print, rather than library's .setText, for smoother display (in this case)
  Serial2.print(_degC);  
  Serial2.print("\"");
  Serial2.write(0xFF);  
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}

/* 
void ControlPanel::UpdateTemp(double temp){     //Updates _tempArray
  int elapsedTime = second();
  _oldArrayIC = _arrayIndexCounter;
  if (elapsedTime - _graphInterval > 5){
    _graphInterval = elapsedTime;
    _tempArray[_arrayIndexCounter] = int(temp);
    _arrayIndexCounter += 1;
  }
}


void ControlPanel::PlotTemp(int counter){
  if (counter == 0){
    for (int i=0; i<_arrayIndexCounter; i++){
      Serial2.print("add 1,0,");
      Serial2.print(_tempArray[_arrayIndexCounter]);
      Serial2.write(0xFF);
      Serial2.write(0XFF);
      Serial2.write(0xFF);
    }
  } 
  if (_oldArrayIC != _arrayIndexCounter){
    Serial2.print("add 1,0,");
    Serial2.print(_tempArray[_arrayIndexCounter]);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
  }
  vTaskDelay(3);
}
*/

void ControlPanel::DisplayProgress(){                               //Updates progress bar of session
  if (this->_sessionState == 1 && this->_oldSessionState == 0 
                               && this->_onGoingSession == false){  //Session start
    _oldSessionState = 1;
    cb0.getText(_sessionTimeCBox,2);
    this->_sessionTime = atoi(_sessionTimeCBox);
    this->_startTime = minute();
    
  } else if (this->_sessionState == 1){
    _thisMinute = minute() - _pausedMinute;
    if (_thisMinute - this->_startTime > this->_sessionTime ){      //Session Ended
      LEDOff();
      this->_sessionTime = 0;
      this->_startTime = 0;
      this-> _onGoingSession = false;
      
    } else {                                                        //Session Ongoing
      this->_onGoingSession = true;  
      _progressBar = ((_thisMinute - double(this->_startTime))/double(this->_sessionTime))*100;
      j0.setValue(uint32_t(_progressBar));
      
      Serial2.print("t1.txt=\"");
      Serial2.print(int(_thisMinute - double(this->_startTime)));
      Serial2.print(" min | ");
      Serial2.print(_progressBar);
      Serial2.print("%");
      Serial2.print("\"");
      Serial2.write(0xFF);
      Serial2.write(0xFF);
      Serial2.write(0xFF);
    }
  } else if (this->_sessionState == 0 && this->_oldSessionState == 1 
                                      && this->_onGoingSession == true){   //Pause Session
    this->_pausedMinute = minute() - _thisMinute;
  }
  vTaskDelay(100);
}


int ControlPanel::CurrentPage(){                              //Returns the current page
  return _pg;
}


void ControlPanel::LEDIndex(){                  //LED output control
  //LED Driver runs on inverse logic (5V - LOW)
  if (this->_sessionState == 1){
    CurrentInt();
    GetSessionTime();

    if (this->_sessionInt == 0){         
      LowMode();
    } else if (this->_sessionInt == 1){
      MedMode();
    } else if (this->_sessionInt == 2){
      HighMode();
    }
  } else {
    LEDOff();
  }
}

void ControlPanel::CurrentInt(){                 //Gets the current session intentsity
  uint32_t sessionIntensity;
  cb0.getValue(&sessionIntensity);
  this->_sessionInt = sessionIntensity;
}


void ControlPanel::StartSession(){                //Starts the session
  uint32_t startButton;
  sw20.getValue(&startButton);
  _oldSessionState = _sessionState;
  _sessionState = startButton? 1 : 0;
}


int ControlPanel::GetSessionTime(){               //Returns the selected session time
  uint32_t sessionTime;
  cb0.getValue(&sessionTime);
  this->_sessionTime = sessionTime;
  return this->_sessionTime;
}


void ControlPanel::PrevSelection(){               //Restores the previous button selections when returning to the same page in a single session
    if (_pg == 2){
      sw20.setValue(_sessionState);               //Start button
      cb0.setValue(this->_sessionTime);           //Session time combobox
      cb1.setValue(this->_sessionInt);            //Session intensity combobox
    }
}


void ControlPanel::LowMode(){                    
  //Driver runs on inverse logic (5V - LOW)
  digitalWrite(_DRIVER1, LOW);
  delayMicroseconds(3330);
  digitalWrite(_DRIVER1,HIGH);
  digitalWrite(_DRIVER2,LOW);
  delayMicroseconds(3330);
  digitalWrite(_DRIVER2,HIGH);
  digitalWrite(_DRIVER3,LOW);
  delayMicroseconds(3330);
  LEDOff();
  delay(30);                                      //To minus 1 if vTaskDelay is added in secondcore
}


void ControlPanel::MedMode(){
  //Driver runs on inverse logic (5V - LOW)
  digitalWrite(_DRIVER1, LOW);
  delayMicroseconds(4500);
  digitalWrite(_DRIVER1,HIGH);
  digitalWrite(_DRIVER2,LOW);
  delayMicroseconds(4500);
  digitalWrite(_DRIVER2,HIGH);
  digitalWrite(_DRIVER3,LOW);
  delayMicroseconds(4500);
  LEDOff();
  delayMicroseconds(26500);                     //To minus 100 if vTaskDelay is added in secondcore
}


void ControlPanel::HighMode(){
  //Driver runs on inverse logic (5V - LOW)
  digitalWrite(_DRIVER1, LOW);
  delayMicroseconds(6670);
  digitalWrite(_DRIVER1,HIGH);
  digitalWrite(_DRIVER2,LOW);
  delayMicroseconds(6670);
  digitalWrite(_DRIVER2,HIGH);
  digitalWrite(_DRIVER3,LOW);
  delayMicroseconds(6670);
  LEDOff();
  delay(20);                                    //To minus 1 if vTaskDelay is added in secondcore
}

void ControlPanel::LEDOff(){                      //Turns off all LEDs
  //LED Driver runs on inverse logic (5V - LOW)
  digitalWrite(_DRIVER1, HIGH);
  digitalWrite(_DRIVER2, HIGH);
  digitalWrite(_DRIVER3, HIGH);
}


void ControlPanel::LEDOn(){                       //Turns on all LEDs
  //LED Driver runs on inverse logic (5V - LOW)
  digitalWrite(_DRIVER1, LOW);
  digitalWrite(_DRIVER2, LOW);
  digitalWrite(_DRIVER3, LOW);
}
