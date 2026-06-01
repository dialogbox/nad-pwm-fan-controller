#pragma once
#include "config.h"

extern bool isKickstarting;
extern unsigned long kickstartEndTime;
extern const unsigned long kickstartDuration;

void triggerKickstart();
void updateFan();
