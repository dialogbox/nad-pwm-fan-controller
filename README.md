# Smart PWM Fan Controller for NAD M33 / BluOS Streamers

A modular, robust, and highly optimized ESP8266-based cooling controller for premium BluOS music streamers (like the NAD M33). The system dynamically adjusts fan speeds based on the streamer's power state, stores configurations persistently in EEPROM, and hosts a state-of-the-art interactive Web UI with 5-minute real-time temperature graphing.

---

## Key Features

* 🤫 **Silent 25kHz Ultrasonic PWM**: Reconfigures the ESP8266 PWM clock frequency to 25kHz (completely ultrasonic) to eliminate low-speed fan coil whine, motor vibration, and humming.
* ⚡ **High-Torque Kickstart Loop**: Automatically pulses the fan to 100% duty cycle for 800ms whenever transitioning from a stopped state, guaranteeing the fan overcomes starting friction at low speeds.
* 💾 **Persistent EEPROM Configuration**: Stores power mode, target fan speed, target BluOS network endpoint, and temperature monitoring settings securely in non-volatile memory with validation checks.
* 📉 **Interactive Chart.js Visualizations**: Replaces manual SVG drawing with Chart.js, rendering a responsive multi-sensor temperature timeline. Supports index-based hover tooltips, smooth cubic interpolation, and toggles.
* 🔄 **Telemetry Sustaining**: Prevents chart line breaks by automatically sustaining the last known temperature value for sensors that report telemetry at varying intervals.
* 💡 **Standby & Failsafe Overlay**: Displays a sleek dashed "Power is Off" overlay over a faded background when the BluOS device is unreachable or enters standby.
* 🚀 **Zero-Heap Flash Server (PROGMEM)**: Serves the Web UI template directly out of ESP8266 Flash memory rather than RAM, **reducing dynamic RAM usage from 64.4% to just 37.0%** to completely eliminate out-of-memory crashes on page refresh.

---

## System Architecture

The codebase follows a strictly modular, single-responsibility file layout:

```text
nad-pwm-fan-controller/
├── include/
│   ├── config.h               # Global state, structures, and shared variables
│   ├── storage.h              # EEPROM configuration read/write definitions
│   ├── fan.h                  # Fan ramping and ultrasonic PWM orchestration
│   ├── bluos.h                # Telnet socket parser and query state machine
│   ├── webui.h                # Web server endpoints and HTML/JS template
│   └── wifi_credentials.h     # Secret WiFi credentials (GIT-IGNORED)
├── src/
│   ├── main.cpp               # System entry point, hardware init, and main loop
│   ├── storage.cpp            # Non-volatile EEPROM storage load/save mechanics
│   ├── fan.cpp                # Ultrasonic ramping, motor kickstarting, and constraints
│   ├── bluos.cpp              # Telnet stream listener, power event decoder, and state queries
│   └── webui.cpp              # HTTP page routing, status payload assembler, and PROGMEM serving
├── platformio.ini             # PlatformIO build configuration and hardware profiles
└── README.md                  # Developer manual and operations walkthrough
```

---

## Configuration & Credentials Security

WiFi credentials (SSID and Password) are kept out of source control entirely by storing them in a gitignored header file.

### Initial Setup
1. Locate the public template: `include/wifi_credentials.h.template`.
2. Duplicate or rename the template to: `include/wifi_credentials.h`.
3. Open `include/wifi_credentials.h` and enter your WiFi details:
   ```cpp
   #pragma once
   #define WIFI_SSID "YOUR_SSID"
   #define WIFI_PASS "YOUR_PASSWORD"
   ```
This file is automatically ignored by Git (configured in `.gitignore`), preventing accidental leakage of passwords.

---

## Operations & Deployment

The project is configured for **PlatformIO**.

### 1. Verification & Compilation
Run the local compiler to check for syntax and binary constraints:
```bash
pio run
```

### 2. Uploading Firmware
Connect your ESP8266 board (e.g. NodeMCU 1.0) over USB-C. The build profile auto-detects the active port (typically `/dev/ttyUSB1` or `/dev/ttyUSB0`) and uploads the binary:
```bash
pio run -t upload
```

*Note: Ensure any active Serial monitors (such as `pio device monitor`) are closed before uploading, as they will lock the serial port.*

---

## Logging & Telemetry Signal

High-frequency temperature telemetry lines (`Main.Temp.*` occurring at 1Hz) are filtered out from the Serial output to prevent log pollution. The Serial port outputs clean, high-signal action logs:

```text
[EEPROM] Settings loaded successfully
[EEPROM] Mode: 2 | Speed: 120 | Endpoint: 192.168.0.18:23 | TempMon: 1
WiFi Connected!
IP Address: http://192.168.0.50
[Web] HTTP Web Server initialized

[BluOS] Attempting connection to 192.168.0.18:23
[BluOS] Connected successfully!
[BluOS TX] Main.Power? (State query)
[BluOS RX] Main.Model=M33
[BluOS RX] Main.Power=On
[BluOS Event] Device Powered ON!
[Fan] Kick-starting motor (100% PWM)

[Web] GET / (Root UI Page Requested)
[Web] GET /toggle?state=2 (Power mode changed from 1 to 2)
[EEPROM] Configuration saved successfully
```
