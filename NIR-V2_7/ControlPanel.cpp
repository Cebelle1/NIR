/*
 * ControlPanel.h - Library used for NIR's Control Panel.
 * Created by Loo S H, April 4, 2022.
 */

#include <TimeLib.h>
#include "ControlPanel.h"
#include <Arduino.h>
#include <Nextion.h>

//To store the state of buttons etc.
int ControlPanel::_pg = 0;
int ControlPanel::_sessionState = 0;
int ControlPanel::_stateArray[3] = {0, 0, 0};
int ControlPanel::_sessionTime = 0;
int ControlPanel::_startTime = 0;
int ControlPanel::_oldSessionState = 0;
bool ControlPanel::_onGoingSession = false;
double ControlPanel::_pausedMinute = 0;
int ControlPanel::_frequency = 60;


ControlPanel::ControlPanel(int RELAY1, int RELAY2, int RELAY3, int RELAY4, int PWM1, int PWM2, int PWM3){
  _RELAY1 = RELAY1;
  _RELAY2 = RELAY2;
  _RELAY3 = RELAY3;
  _RELAY4 = RELAY4;
  _PWM1 = PWM1;
  _PWM2 = PWM2;
  _PWM3 = PWM3;
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


void ControlPanel::Display_TempDist(double temp, uint8_t range){    //Display temperature and distance
  _degC = String(temp);
  _range = String(range);
  
  Serial2.print("t2.txt=\"");   //Use Serial2.print, rather than library's .setText, for smoother display (in this case)
  Serial2.print(_degC);  
  Serial2.print("\"");
  Serial2.write(0xFF);  
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  
  Serial2.print("t3.txt=\"");  
  Serial2.print(_range);  
  Serial2.print("\"");
  Serial2.write(0xFF);  
  Serial2.write(0xFF);
  Serial2.write(0xFF);
 
}


void ControlPanel::DisplayProgress(){             //Updates progress bar of session
  
  if (this->_sessionState == 1 && this->_oldSessionState == 0 && this->_onGoingSession == false){ //Session start
    _oldSessionState = 1;
    cb0.getText(_sessionTimeCBox,2);
    this->_sessionTime = atoi(_sessionTimeCBox);
    this->_startTime = minute();
    
  } else if (this->_sessionState == 1){
    _thisMinute = minute() - _pausedMinute;
    if (_thisMinute - this->_startTime > this->_sessionTime ){  //Session Ended
      LEDOff();
      this->_sessionTime = 0;
      this->_startTime = 0;
      //_progressBar = 0;
      //j0.setValue(uint32_t(0));
      
    } else {  //Session Ongoing
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
  } else if (this->_sessionState == 0 && this->_oldSessionState == 1 && this->_onGoingSession == true){   //Pause Session
    this->_pausedMinute = minute() - _thisMinute;
  }

  vTaskDelay(100);
}


int ControlPanel::CurrentPage(){                    //Returns the current page
  return _pg;
}


void ControlPanel::LEDIndex(int startState){      //LED output control
  //Relay Module uses Low Level Trigger (Inverse Logic)
  if (startState){
    if (_stateArray[0]){
      digitalWrite(_RELAY1, LOW);
    } else {
      digitalWrite(_RELAY1, HIGH);
    }
    
    if (_stateArray[1]){
      digitalWrite(_RELAY2, LOW);
    } else {
      digitalWrite(_RELAY2, HIGH);
    }
    
    if (_stateArray[2]){
      digitalWrite(_RELAY3, LOW);
    } else {
      digitalWrite(_RELAY3, HIGH);
    }
  } else {
    LEDOff();
  }
}


void ControlPanel::CurrentInt(){                 //Gets the current switch state and updates stateArray
  uint32_t dual_state;
  
  sw0.getValue(&dual_state);
  _stateArray[0] = dual_state;
  sw1.getValue(&dual_state);
  _stateArray[1] = dual_state;
  sw2.getValue(&dual_state);
  _stateArray[2] = dual_state;

}


void ControlPanel::LEDFreq(){                   //LED PWM Control (Running on second core)
  _period = (1/this->_frequency) * 1000;

  if (_stateArray[0]){
    digitalWrite(_PWM1, HIGH);
    delayMicroseconds(_period);
    digitalWrite(_PWM1, LOW);
    delayMicroseconds(_period);
  } else {
    digitalWrite(_PWM1, LOW);
  }

  if(_stateArray[1]){
    digitalWrite(_PWM2, HIGH);
    delayMicroseconds(_period);
    digitalWrite(_PWM2, LOW);
    delayMicroseconds(_period);
  } else {
    digitalWrite(_PWM2, LOW);
  }

  if (_stateArray[2]){
    digitalWrite(_PWM3, HIGH);
    delayMicroseconds(_period);
    digitalWrite(_PWM3, LOW);
    delayMicroseconds(_period);
  } else {
    digitalWrite(_PWM3, LOW);
  }
}


void ControlPanel::FreqControl(int freqCon){    //FreqBox (text) display
  
  _freq = CurrentFreq();

  if (freqCon == 0 && _freq > 10){
    _freq -= 10;
  } else if (freqCon == 1 && _freq < 80){
    _freq += 10;
  }

  this->_frequency = _freq;  
  Serial2.print("FreqBox.txt=\"");  
  Serial2.print(_freq); 
  Serial2.print("\"");
  Serial2.write(0xFF); 
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}


int ControlPanel::CurrentFreq(){              //Returns the current frequency in hertz
  
  char freq_box[2];
  FreqBox.getText(freq_box,2);
  _getFreq =atoi(freq_box);
  return _getFreq;
}



void ControlPanel::StartSession(){            //Starts the session
  uint32_t startButton;
  
  sw20.getValue(&startButton);
  _oldSessionState = _sessionState;
  if(startButton){
    LEDIndex(1);
    _sessionState = 1;
  } else {
    LEDIndex(0);
    _sessionState = 0;
  }
}


int ControlPanel::GetSessionTime(){           //Returns the selected session time
  char sessionTime[2];
  cb0.getText(sessionTime,2);
  this->_sessionTime = atoi(sessionTime);
  return _sessionTime;
}


void ControlPanel::PrevSelection(){           //Restores the previous button selections when returning to the same page in a single session
  
    if (_pg == 1){
      sw0.setValue(_stateArray[0]);
      sw1.setValue(_stateArray[1]);
      sw2.setValue(_stateArray[2]);
      Serial2.print("FreqBox.txt=\"");
      Serial2.print(_frequency);
      Serial2.print("\"");
      Serial2.write(0xFF);
      Serial2.write(0xFF);
      Serial2.write(0xFF);
    } else if (_pg == 2){
      sw20.setValue(_sessionState);
      if(this->_sessionTime == 25){
        cb0.setValue(0);
      }else if (this->_sessionTime == 35){
        cb0.setValue(1);
      }else if (this->_sessionTime == 45){
        cb0.setValue(2);
      }
    }
}


void ControlPanel::LEDOff(){                  //Turns off all LEDs
/*
 * _RELAY4 is not used,
 * SSR modules runs on Low Level Trigger (Inverse Logic)
 */
  digitalWrite(_RELAY1, HIGH);
  digitalWrite(_RELAY2, HIGH);
  digitalWrite(_RELAY3, HIGH);
  digitalWrite(_RELAY4, HIGH);
}


void ControlPanel::LEDOn(){                   //Turns on all LEDs
/*
 * _RELAY4 is not used,
 * SSR modules runs on Low Level Trigger (Inverse Logic)
 */
  digitalWrite(_RELAY1, LOW);
  digitalWrite(_RELAY2, LOW);
  digitalWrite(_RELAY3, LOW);
}
