/*
 * UserLogin.h - Library used for handling NIR's user logins. 
 * Created by Loo S H, April 4, 2022.
 */

#include "UserLogin.h"
#include <MFRC522.h>

//Define new cards here and add to UserCardID array. 
#define CEBELLE 132

UserLogin::UserLogin(int SS_PIN, int RST_PIN){
  _ssPin = SS_PIN;
  _rstPin = RST_PIN;
}


int UserLogin::UserCardID[1] = {CEBELLE};     //Add new card defination here


bool UserLogin::UserCard(int _rfid){          //Returns true if card is authorised
  for (int i = 0; i<1; i++){
    if(_rfid == UserCardID[i]){
      return true;
    } else {
      return false;
    }
  }
}


int UserLogin::ReadRFID() {                   //Returns the RFID number
  MFRC522 mfrc522(_ssPin, _rstPin);
  _rfidByte = 0;
  if (!mfrc522.PICC_IsNewCardPresent()) {
    vTaskDelay(100);
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    vTaskDelay(100);
  }
  _rfidByte = mfrc522.uid.uidByte[1];  //Read card value and store into the variable "RFID"
  return _rfidByte;
}
