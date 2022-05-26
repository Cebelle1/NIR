/*
 * ButtonConfig.h - Inheriting Nexion's Library, a library for NIR's screen button configurations.
 * Created by Loo S H, May 26, 2022.
 */

#ifndef ButtonConfig_h
#define ButtonConfig_h

#include <Arduino.h>
#include "ControlPanel.h"

void b0PushCallBack   (void *ptr);
void b1PushCallBack   (void *ptr);
void b20PushCallBack  (void *ptr);
void sw20PopCallBack  (void *ptr);

#endif
