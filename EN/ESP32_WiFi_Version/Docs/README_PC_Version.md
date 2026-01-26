# MojiKit Action Sequence Designer - PC Version

## Overview

This is a robot action sequence designer running on PC, connecting to ESP32 via WiFi and forwarding generated commands to Arduino Mega2560 servo control system.

## System Architecture

```
PC (Web Page) ←→ WiFi ←→ ESP32 ←→ Serial Port ←→ Arduino Mega2560 ←→ Servos
```

## Usage

### 1. Hardware Connection

- ESP32 GPIO16 → Arduino Mega2560 RX1 (19)
- ESP32 GPIO17 → Arduino Mega2560 TX1 (18)  
- ESP32 VIN → Arduino Mega2560 5V
- ESP32 GND → Arduino Mega2560 GND

### 2. Start ESP32

1. Upload `ESP32/ESP32_WiFi_Server.ino` to ESP32
2. ESP32 will create WiFi hotspot:
   - Network Name: `RobotControl`
   - Password: `12345678`
   - IP Address: `192.168.4.1`

### 3. Connect WiFi

1. Connect PC to ESP32 WiFi hotspot `RobotControl`
2. Enter password `12345678`

### 4. Run Web Page

1. After connecting PC to ESP32 WiFi hotspot, visit `http://192.168.4.1` in browser
2. Or open `Web/index.html` file (if downloaded locally)
3. Click "🧪 Test Connection" button to verify connection
4. Click "📡 Connect ESP32" button to establish connection

**Or directly access ESP32 status page:**
- Visit `http://192.168.4.1` in browser to view ESP32 running status

### 5. Design Action Sequence

1. Select robot structures (RF, LF, RR, LR, TA, RE, LE, HD)
2. Set angle parameters and speed
3. Add actions to timeline
4. Play or export command code

## Features

- 🎯 Visual action sequence design
- ⏰ Timeline editor
- 🤖 Robot 3D preview
- 📡 WiFi real-time control
- 💾 Command code export
- 🔄 Real-time status monitoring

## Command Format

```
Structure FAngle RAngle SSpeed DDelay
```

Examples:
- `RF F30 R20 S2` - Right front leg, forward 30°, bend 20°, speed level 2
- `LF F-30 R15 S2 D500` - Left front leg, forward -30°, bend 15°, speed level 2, delay 500ms

## Troubleshooting

### Connection Issues
1. Ensure connected to ESP32 WiFi hotspot
2. Check if ESP32 is running normally
3. Use "Test Connection" button to verify connection

### Command Send Failed
1. Check WiFi connection status
2. Confirm ESP32 to Arduino serial connection
3. Check browser console error messages

### Cross-Origin Resource Sharing (CORS) Issues
If encountering "Failed to fetch" or CORS errors:
1. Ensure ESP32 code includes CORS header support
2. Re-upload ESP32 firmware
3. Check browser console for CORS-related errors
4. Some browsers may need security restrictions disabled (for development testing only)

## File Description

- `ESP32/ESP32_WiFi_Server.ino` - ESP32 firmware, acts as WiFi hotspot and command forwarder
- `Arduino/Arduino2560control.ino` - Arduino Mega2560 program (WiFi version)
- `Web/index.html` - ESP32 server web interface, action sequence designer
- `README_PC_Version.md` - This documentation

## Notes

- Ensure PC and ESP32 are on the same WiFi network
- ESP32 IP address is fixed at 192.168.4.1
- Web page needs to run in modern browser environment
- Recommend using Chrome, Firefox, or Edge browser

