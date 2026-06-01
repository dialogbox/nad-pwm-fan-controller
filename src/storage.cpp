#include "storage.h"
#include <EEPROM.h>

extern EEPROM_Data eepromData;

void loadSettings() {
  EEPROM.begin(sizeof(EEPROM_Data));
  EEPROM.get(0, eepromData);
  
  if (eepromData.magic != 0xABCD1235) {
    eepromData.magic = 0xABCD1235;
    eepromData.powerMode = 0;
    eepromData.sliderSpeed = 80;
    strncpy(eepromData.endpoint, "192.168.0.18:23", sizeof(eepromData.endpoint));
    eepromData.tempMonitorEnabled = 0;
    
    EEPROM.put(0, eepromData);
    EEPROM.commit();
    Serial.println("[EEPROM] Initialized default configurations");
  } else {
    Serial.println("[EEPROM] Settings loaded successfully");
  }
  
  powerMode = eepromData.powerMode;
  sliderSpeed = eepromData.sliderSpeed;
  bluosEndpoint = String(eepromData.endpoint);
  tempMonitorEnabled = (eepromData.tempMonitorEnabled != 0);
  
  Serial.printf("[EEPROM] Mode: %d | Speed: %d | Endpoint: %s | TempMon: %d\n", powerMode, sliderSpeed, bluosEndpoint.c_str(), tempMonitorEnabled);
}

void saveSettings() {
  eepromData.powerMode = powerMode;
  eepromData.sliderSpeed = sliderSpeed;
  strncpy(eepromData.endpoint, bluosEndpoint.c_str(), sizeof(eepromData.endpoint));
  eepromData.tempMonitorEnabled = tempMonitorEnabled ? 1 : 0;
  
  EEPROM.put(0, eepromData);
  EEPROM.commit();
  Serial.println("[EEPROM] Configuration saved successfully");
}
