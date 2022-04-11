/*
 * ControlPanel.h - Library used for NIR's Control Panel.
 * Created by Loo S H, April 4, 2022.
 */

#ifndef ControlPanel_h
#define ControlPanel_h
 
#include <Arduino.h>
#include <Nextion.h>
#include <TimeLib.h>


class ControlPanel {
  NexDSButton sw0 = NexDSButton(1,6,"sw0");                   //625nm LED   
  NexDSButton sw1 = NexDSButton(1,7,"sw1");                   //850nm LED
  NexDSButton sw2 = NexDSButton(1,8,"sw2");                   //940nm LED
  NexDSButton cb0 = NexDSButton(2,2,"cb0");                   //Session time  
  NexText FreqBox = NexText(1,1, "FreqBox");
  NexDSButton sw20 = NexDSButton(2,3,"sw0");                  //Start Session
  NexText TimeBox = NexText(2,6,"t1");
  NexText DegBox = NexText(1,3,"t2");
  NexText DistBox = NexText(1,3,"t3");
  NexProgressBar j0 = NexProgressBar(2,4,"j0");
  
  public: 
    ControlPanel(int RELAY1, int RELAY2, int RELAY3, int RELAY4, int PWM1, int PWM2, int PWM3);
    
    void DisplayPage(int pg);                                 //Change page
    void Display_TempDist(double temp, uint8_t range);        //Display temperature and distance
    void DisplayProgress();                                   //Updates progress bar of session
    static int CurrentPage();                                 //Returns the current page
    
    void LEDIndex(int startState);                            //LED output control
    void CurrentInt();                                        //Gets the current switch state (sw0-sw2) and updates stateArray
    
    void LEDFreq();                                           //LED PWM Control (Running on second core)
    void FreqControl(int freqCon);                            //FreqBox (text) display
    int CurrentFreq();                                        //Returns the current selected frequency in hertz
    
    void StartSession();                                      //Starts the session
    int GetSessionTime();                                     //Returns the selected session time
    void PrevSelection();                                     //Restores the previous button selections when returning to the same page in a single session

    void LEDOff();                                            //Turns off all LEDs
    void LEDOn();                                             //Turns on all LEDs

  private: 
    //Static variables are used to store states throughout the runtime
    static int _pg;                     //Stores the current page
    static int _frequency;              //Stores the selected frequency
    static int _sessionTime;            //Stores the remaining time to run for the session
    static int _startTime;              //Stores the time when session starts
    static int _oldSessionState;        //Stores the previous state of the start button
    static int _stateArray[3];          //Stores the state of the LED switches    (ON/OFF for each LED)
    static int _sessionState;           //Stores the state of the start button (ON/OFF)              
    static bool _onGoingSession;        //Stores the state of the current session (PAUSED/STOPPED)
    static double _pausedMinute;        //Stores the amount of time past since pause 

    String _stringPage = "page page";   //Used in change page command for Nextion
    String _range, _degC;               //Used for type conversion to pass to Serial2
    char _sessionTimeCBox[2];           //Used for retrieving the session time combobox
    
    int _freq;
    int _getFreq;
    int _period;
    int _RELAY1, _RELAY2, _RELAY3, _RELAY4;
    int _PWM1, _PWM2, _PWM3;
    int _counter;
    double _progressBar;                //Displays status of progress bar ontop of progress bar
    double _thisMinute;                 //Displays session time in minute ontop of progress bar
   
  };

#endif
