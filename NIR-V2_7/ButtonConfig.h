/*
 * ButtonConfig.h - Inheriting Nexion's Library, a library for NIR's screen button configurations.
 * Created by Loo S H, April 4, 2022.
 */

#ifndef ButtonConfig_h
#define ButtonConfig_h

#include <Arduino.h>
#include "ControlPanel.h"

void b0PushCallBack(void *ptr);
void b1PushCallBack(void *ptr);
void b10PushCallBack(void *ptr);
void b11PushCallBack(void *ptr);
void b12PushCallBack(void *ptr);
void sw0PopCallBack(void *ptr);
void sw1PopCallBack(void *ptr);
void sw2PopCallBack(void *ptr);
void b20PushCallBack(void *ptr);
void cb0PopcallBack(void *ptr);
void sw20PopCallBack(void *ptr);

#endif
