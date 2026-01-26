# MojiKit Robot Action Sequence Designer - ESP32 WiFi Version

## Overview

This is an ESP32 WiFi-based robot action sequence designer that allows users to design robot action sequences through WiFi connection and send the generated code to Arduino Mega2560 through ESP32 via serial port.

## Hardware Connection

### ESP32 to Arduino Mega2560 Connection
- ESP32 TX (GPIO17) → Arduino Mega2560 RX1 (19)
- ESP32 RX (GPIO16) → Arduino Mega2560 TX1 (18)
- ESP32 VIN → Arduino Mega2560 5V
- ESP32 GND → Arduino Mega2560 GND

### Serial Configuration
- Baud Rate: 115200
- Data Bits: 8
- Stop Bits: 1
- No Parity

## Usage

### 1. Connect WiFi
1. Search for WiFi network on PC
2. Connect to WiFi hotspot named `RobotControl`
3. Enter password: `12345678`

### 2. Access Web Page
1. Open browser
2. Visit `http://192.168.4.1`
3. Page will display robot action sequence designer

### 3. Connect ESP32
1. Click "📡 Connect ESP32" button on the page
2. System will automatically detect ESP32 connection status
3. After successful connection, status display will change to "ESP32 Connected"

### 4. Design Action Sequence
1. Select robot structures (left ear, right ear, head, legs, etc.)
2. Set angle parameters
3. Add actions to timeline
4. Adjust action timing

### 5. Send Code
1. Click "Play Sequence" button to start execution
2. System will automatically send generated code to ESP32
3. ESP32 forwards code to Arduino Mega2560 via serial port
4. ESP32 responses can be seen in code output area

## Features

- **WiFi Connection**: Connect through ESP32 WiFi hotspot, no USB cable needed
- **Real-time Communication**: Supports real-time command sending and response receiving
- **Status Monitoring**: Automatically monitors connection status, auto-reconnect on disconnection
- **Code Generation**: Automatically generates Arduino code and sends for execution
- **Visual Design**: Intuitive graphical interface for designing robot actions

## Technical Architecture

```
PC Browser ←→ ESP32 WiFi Hotspot ←→ ESP32 Web Server ←→ ESP32 Serial Port ←→ Arduino Mega2560
```

### ESP32 Functions
- Create WiFi hotspot
- Provide Web server
- Forward HTTP commands to serial port
- Monitor serial port responses

### Web Page Functions
- Robot action sequence design
- WiFi connection management
- Real-time command sending
- Response status display

## Troubleshooting

### Cannot Connect to WiFi
- Check if ESP32 started normally
- Confirm WiFi name and password are correct
- Restart ESP32

### Cannot Access Web Page
- Confirm connected to correct WiFi network
- Check if browser address is correct
- Try refreshing page

### Command Send Failed
- Check ESP32 to Arduino connection
- Confirm serial configuration is correct
- Check ESP32 serial output

### Connection Status Abnormal
- Click disconnect then reconnect
- Check if ESP32 is running normally
- Restart ESP32

## Development Notes

### Modify WiFi Configuration
In `ESP32/ESP32_WiFi_Server.ino`:
```cpp
const char* ssid = "Your WiFi Name";
const char* password = "Your WiFi Password";
```

### Modify Serial Configuration
In `ESP32/ESP32_WiFi_Server.ino`:
```cpp
#define SERIAL_BAUD_RATE Your Baud Rate
#define ARDUINO_RX_PIN Your RX Pin
#define ARDUINO_TX_PIN Your TX Pin
```

### Add New API Endpoints
Add new route handlers in `setupWebServer()` function.

## Version History

- v1.0: Initial version, supports basic WiFi connection and command forwarding
- Supports robot action sequence design
- Supports real-time command sending and response display

## License

This project is licensed under MIT License. See LICENSE file for details.

