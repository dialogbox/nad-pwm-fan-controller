#pragma once
#include "config.h"

extern WiFiClient bluosClient;
extern unsigned long lastConnectAttempt;
extern const unsigned long connectInterval;

void handleBluOS();
