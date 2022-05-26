/*
 * ControlPanel.h - Library used for NIR's Control Panel.
 * Created by Loo S H, May 26, 2022.
 */

#ifndef ControlPanel_h
#define ControlPanel_h
 
#include <Arduino.h>
#include <Nextion.h>
#include <TimeLib.h>


class ControlPanel {
  NexDSButton cb0 = NexDSButton(2,2,"cb0");                   //Session time  
  NexDSButton cb1 = NexDSButton(2,8,"cb1");                   //Session intensity
  NexDSButton sw20 = NexDSButton(2,3,"sw0");                  //Start Session
  NexText TimeBox = NexText(2,6,"t1");                        //Session time text
  NexProgressBar j0 = NexProgressBar(2,4,"j0");               //Session progress bar
  
  NexText DegBox = NexText(5,3,"TempBox");
  NexText DistBox = NexText(0,2,"DistBox");
  
  public: 
    ControlPanel(int DRIVER1, int DRIVER2, int DRIVER3);
    
    void DisplayPage(int pg);                                 //Change page
    void DisplayDist(uint8_t range);                          //Display distance
    void DisplayTemp(double temp);                            //Display temperature
    void UpdateTemp(double temp);                             //Updates _tempArray
    void PlotTemp(int counter);                               //Plot the temperature grah
    void DisplayProgress();                                   //Updates progress bar of session
    static int CurrentPage();                                 //Returns the current page
    
    void LEDIndex();                            //LED output control
    void CurrentInt();                                        //Gets the current switch state (sw0-sw2) and updates stateArray
    
    void StartSession();                                      //Starts the session
    int GetSessionTime();                                     //Returns the selected session time
    void PrevSelection();                                     //Restores the previous button selections when returning to the same page in a single session

    void LowMode();                                           //Low intensity mode
    void MedMode();                                           //Medium intensity mode
    void HighMode();                                          //High intensity mode
    void LEDOff();                                            //Turns off all LEDs
    void LEDOn();                                             //Turns on all LEDs

  private: 
    //Static variables are used to store states throughout the runtime
    static int _pg;                     //Stores the current page
    static int _sessionTime;            //Stores the remaining time to run for the session
    static int _sessionInt;             //Stores the intensity for the current session
    static int _startTime;              //Stores the time when session starts
    static int _oldSessionState;        //Stores the previous state of the start button    
    static int _sessionState;           //Stores the state of the start button (ON/OFF)        
    static bool _onGoingSession;        //Stores the state of the current session (PAUSED/STOPPED)
    static double _pausedMinute;        //Stores the amount of time past since pause 
    static int _tempArray[600];         //Stores temperature
    static int _arrayIndexCounter;  
    static int _oldArrayIC;

    String _stringPage = "page page";   //Used in change page command for Nextion
    String _range, _degC;               //Used for type conversion to pass to Serial2
    char _sessionTimeCBox[2];           //Used for retrieving the session time combobox
    
    int _freq;
    int _getFreq;
    int _period;
    int _DRIVER1, _DRIVER2, _DRIVER3;
    int _PWM1, _PWM2, _PWM3;
    int _counter;
    double _progressBar;                //Displays status of progress bar ontop of progress bar
    double _thisMinute;                 //Displays session time in minute ontop of progress bar   
    int _graphInterval;            
  };

#endif
