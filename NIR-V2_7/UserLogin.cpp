/*
 * UserLogin.h - Library used for handling user logins. 
 * Created by Loo S H, April 4, 2022.
 */

#include "UserLogin.h"
#include <MFRC522.h>

#define CEBELLE 132

UserLogin::UserLogin(int SS_PIN, int RST_PIN){
  _ssPin = SS_PIN;
  _rstPin = RST_PIN;
}

int UserLogin::UserCardID[1] = {CEBELLE};

bool UserLogin::UserCard(int _rfid){
  for (int i = 0; i<1; i++){
    if(_rfid == UserCardID[i]){
      return true;
    } else {
      return false;
    }
  }
}

int UserLogin::ReadRFID() {
  MFRC522 mfrc522(_ssPin, _rstPin);
  _rfidByte = 0;
  if (!mfrc522.PICC_IsNewCardPresent()) {
    vTaskDelay(100);
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    vTaskDelay(100);
  }
  _rfidByte = mfrc522.uid.uidByte[1];  //Read card value and store into the variable "RFID"
  //Serial.print("Reading=");
  //Serial.println(_rfidByte);
  return _rfidByte;
}
