/*
 * UserLogin.h - Library used for handling user logins. 
 * Created by Loo S H, April 4, 2022.
 */

#ifndef UserLogin_h
#define UserLogin_h

#include <Arduino.h>
#include <MFRC522.h>

class UserLogin {
  public:
    UserLogin(int SS_PIN, int RST_PIN);
    static int UserCardID[1];
    bool UserCard(int rfid);
    int ReadRFID();
  private:
    int _ssPin;
    int _rstPin;
    int _yellow;
    int _rfid;
    int _rfidByte;
  
};

#endif
