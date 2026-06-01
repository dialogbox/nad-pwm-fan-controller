#include "config.h"
#include "storage.h"
#include "fan.h"
#include "bluos.h"
#include "webui.h"
#include "wifi_credentials.h"

// Define Global Configurations
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
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


void setup() {
  Serial.begin(115200);
  
  // Clear the terminal screen on boot
  Serial.write("\033[2J");
  Serial.write("\033[H");
  
  pinMode(fanPin, OUTPUT);
  analogWriteFreq(25000); // Set PWM frequency to 25kHz (Ultrasonic range, silent fan standard)
  analogWrite(fanPin, 0); // Start turned off

  // Initialize Storage (EEPROM load)
  loadSettings();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Initialize Web Server
  initWebServer();
}

void loop() {
  server.handleClient(); // Handle inbound HTTP client calls
  handleBluOS();         // Manage Telnet client monitoring state machine
  updateFan();           // Manage fan ramping loop
}
