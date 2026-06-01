#include "bluos.h"
#include "fan.h"

WiFiClient bluosClient;
unsigned long lastConnectAttempt = 0;
const unsigned long connectInterval = 8000; // Connection retry rate

void handleBluOS() {
  if (powerMode != 2 && !tempMonitorEnabled) {
    if (bluosClient.connected()) {
      bluosClient.stop();
      Serial.println("[BluOS] Connection closed (Monitoring disabled)");
    }
    return;
  }

  // Handle reconnect if disconnected
  if (!bluosClient.connected()) {
    if (lastBluOSConnected) {
      Serial.println("[BluOS] Connection lost!");
      lastBluOSConnected = false;
      bluosPowerOn = false;
    }

    unsigned long now = millis();
    if (now - lastConnectAttempt >= connectInterval) {
      lastConnectAttempt = now;
      
      String ipStr = "";
      int port = 23;
      int colonIdx = bluosEndpoint.indexOf(':');
      if (colonIdx != -1) {
        ipStr = bluosEndpoint.substring(0, colonIdx);
        port = bluosEndpoint.substring(colonIdx + 1).toInt();
      } else {
        ipStr = bluosEndpoint;
      }

      if (ipStr.length() > 0) {
        Serial.print("[BluOS] Attempting connection to ");
        Serial.print(ipStr);
        Serial.print(":");
        Serial.println(port);
        
        bluosClient.setTimeout(400); // 400ms timeout prevents freezing
        if (bluosClient.connect(ipStr.c_str(), port)) {
          Serial.println("[BluOS] Connected successfully!");
          lastBluOSConnected = true;
          bluosClient.setTimeout(5); // Ultra low timeout for fast loop
          
          // Query current power status immediately upon connection
          bluosClient.println("Main.Power?");
          Serial.println("[BluOS TX] Main.Power? (State query)");
        } else {
          Serial.println("[BluOS] Connection failed.");
        }
      }
    }
  }

  // Parse live stream
  if (bluosClient.connected()) {
    while (bluosClient.available() > 0) {
      String line = bluosClient.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        if (!line.startsWith("Main.Temp.")) {
          Serial.println("[BluOS RX] " + line);
        }
        if (line.indexOf("Main.Power=On") != -1) {
          if (!bluosPowerOn) {
            Serial.println("[BluOS Event] Device Powered ON!");
            bluosPowerOn = true;
            triggerKickstart();
          }
        } else if (line.indexOf("Main.Power=Off") != -1) {
          if (bluosPowerOn) {
            Serial.println("[BluOS Event] Device Powered OFF!");
            bluosPowerOn = false;
          }
        } else if (line.startsWith("Main.Temp.")) {
          int eqIdx = line.indexOf('=');
          if (eqIdx != -1) {
            String name = line.substring(10, eqIdx);
            int value = line.substring(eqIdx + 1).toInt();
            
            bool found = false;
            for (int i = 0; i < sensorCount; i++) {
              if (tempSensors[i].name == name) {
                tempSensors[i].value = value;
                tempSensors[i].lastUpdate = millis();
                found = true;
                break;
              }
            }
            if (!found && sensorCount < MAX_SENSORS) {
              tempSensors[sensorCount].name = name;
              tempSensors[sensorCount].value = value;
              tempSensors[sensorCount].lastUpdate = millis();
              sensorCount++;
            }
          }
        }
      }
    }
  }
}
