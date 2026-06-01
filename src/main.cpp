#include "config.h"
#include "storage.h"
#include "fan.h"
#include "bluos.h"
#include "webui.h"
#include <WiFiManager.h>

// Define Global Configurations
const int fanPin = D5;

// Define Global State Variables
uint8_t powerMode = 0;        // 0 = OFF, 1 = ON, 2 = AUTO
int sliderSpeed = 50;         // Target speed set by slider (0 - 255)
int currentSpeed = 0;         // Actual speed applied to physical PWM (0 - 255)
int targetSpeed = 0;          // Ramping target speed

String bluosEndpoint = "192.168.0.18:23";
bool bluosPowerOn = false;    // Last known power state of the BluOS player
bool lastBluOSConnected = false;
bool tempMonitorEnabled = false;

ESP8266WebServer server(80);
EEPROM_Data eepromData; // Persistent storage memory block

// Temperature Monitoring Global Arrays
const int MAX_SENSORS = 10;
TempSensor tempSensors[MAX_SENSORS];
int sensorCount = 0;

// Non-blocking FLASH button check (GPIO 0 / NodeMCU D3)
unsigned long buttonPressStartTime = 0;
bool lastButtonState = HIGH;

void checkPhysicalButton() {
  int buttonState = digitalRead(D3);
  if (buttonState == LOW) { // Button is pressed (active low)
    if (lastButtonState == HIGH) {
      buttonPressStartTime = millis();
      lastButtonState = LOW;
    } else {
      if (millis() - buttonPressStartTime >= 3000) {
        Serial.println("[System] FLASH button held for 3s! Resetting WiFi configurations...");
        WiFiManager wm;
        wm.resetSettings();
        delay(500);
        ESP.restart();
      }
    }
  } else {
    lastButtonState = HIGH;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Clear the terminal screen on boot
  Serial.write("\033[2J");
  Serial.write("\033[H");
  
  pinMode(fanPin, OUTPUT);
  analogWriteFreq(25000); // Set PWM frequency to 25kHz (Ultrasonic range, silent fan standard)
  analogWrite(fanPin, 0); // Start turned off
  
  // Configure the on-board FLASH button (active low)
  pinMode(D3, INPUT_PULLUP);

  // Initialize Storage (EEPROM load)
  loadSettings();

  // Initialize WiFi using WiFiManager
  WiFiManager wm;
  wm.setConfigPortalTimeout(180); // 3-minute timeout prevents stuck portals if router reboots
  
  Serial.println("[WiFi] Auto-connecting or launching setup portal...");
  if (!wm.autoConnect("Smart-Fan-Setup")) {
    Serial.println("[WiFi] Portal timeout or connection failed. Rebooting...");
    delay(3000);
    ESP.restart();
  }
  
  Serial.println("\n[WiFi] Connected successfully!");
  Serial.print("[WiFi] IP Address: http://");
  Serial.println(WiFi.localIP());

  // Initialize Web Server
  initWebServer();
}

void loop() {
  server.handleClient(); // Handle inbound HTTP client calls
  handleBluOS();         // Manage Telnet client monitoring state machine
  updateFan();           // Manage fan ramping loop
  checkPhysicalButton(); // Manage physical factory-reset long press
}
