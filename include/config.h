#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Configuration
extern const char* ssid;
extern const char* password;

// Pin Mapping
extern const int fanPin;

// System Configurations Structure
struct EEPROM_Data {
  uint32_t magic;         // Magic validation
  uint8_t powerMode;      // 0 = OFF, 1 = ON, 2 = AUTO
  uint8_t sliderSpeed;    // 0 - 255
  char endpoint[64];      // "192.168.0.18:23"
  uint8_t tempMonitorEnabled; // 0 = disabled, 1 = enabled
};

// Global State Variables
extern uint8_t powerMode;
extern int sliderSpeed;
extern int currentSpeed;
extern int targetSpeed;
extern String bluosEndpoint;
extern bool bluosPowerOn;
extern bool lastBluOSConnected;
extern bool tempMonitorEnabled;

extern ESP8266WebServer server;

// Temperature Sensor Structure
struct TempSensor {
  String name;
  int value;
  unsigned long lastUpdate;
};

extern const int MAX_SENSORS;
extern TempSensor tempSensors[];
extern int sensorCount;

