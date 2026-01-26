# MojiKit Action Sequence Designer - ESP32 WiFi Version

## 📋 Version Description

**ESP32 WiFi Version** controls the robot via WiFi wireless connection, with ESP32 acting as WiFi hotspot and command forwarder to achieve remote control. Suitable for remote control, demonstrations, and multi-device control scenarios.

## 🔧 Hardware Requirements

### Control Board
- **Arduino Mega2560** × 1 (Main controller)
- **ESP32 Development Board** × 1 (WiFi communication module)

### Servo System
- **Standard PWM Servos** (such as SG90, MG996R, etc.) × 16
- **Servo Power Supply**: External 5V power supply, current capacity ≥5A

### Connection Cables
- USB cable (Arduino Mega2560 to computer, for uploading programs)
- USB cable (ESP32 to computer, for uploading programs)
- Servo connection cables (16 wires)
- Dupont wires (ESP32 to Arduino connection)

## 📐 Hardware Connection

### Arduino Mega2560 Pin Assignment

| Structure Code | Structure Name | Servo Number | Arduino Pin | Function Description |
|---------------|---------------|--------------|-------------|---------------------|
| RF | Right Front Leg | Servo 1, 2 | D30, D31 | Lift/drop, bend |
| LF | Left Front Leg | Servo 3, 4 | D32, D33 | Lift/drop, bend |
| RR | Right Rear Leg | Servo 5, 6 | D34, D35 | Lift/drop, bend |
| LR | Left Rear Leg | Servo 7, 8 | D36, D37 | Lift/drop, bend |
| TA | Tail | Servo 9, 10 | D38, D39 | Left/right swing, up/down bend |
| RE | Right Ear | Servo 11, 12 | D40, D41 | Rotation, pitch |
| LE | Left Ear | Servo 13, 14 | D42, D43 | Rotation, pitch |
| HD | Head | Servo 15, 16 | D44, D45 | Left/right rotation, up/down nod |

### ESP32 to Arduino Mega2560 Connection

| ESP32 Pin | Arduino Mega2560 Pin | Description |
|-----------|---------------------|-------------|
| GPIO16 (RX) | TX1 (18) | ESP32 receives data from Arduino |
| GPIO17 (TX) | RX1 (19) | ESP32 sends data to Arduino |
| VIN | 5V | Power positive (optional, ESP32 can be powered independently) |
| GND | GND | Power negative (must connect) |

### Connection Steps

1. **Servo Connection** (Same as PC version)
   - Connect 16 servos to Arduino Mega2560 according to pin assignment table
   - Connect servo signal wires to corresponding digital pins
   - Connect servo power to external 5V power supply

2. **ESP32 to Arduino Connection**
   - ESP32 GPIO16 → Arduino TX1 (18)
   - ESP32 GPIO17 → Arduino RX1 (19)
   - ESP32 GND → Arduino GND
   - ESP32 VIN → Arduino 5V (optional, ESP32 can be powered independently)

3. **Power Connection**
   - Provide external 5V power supply for servos
   - Arduino Mega2560 powered via USB or external power supply
   - ESP32 can be powered via Arduino's 5V or independently

## 🚀 Software Setup

### 1. Upload Arduino Program

1. Open Arduino IDE
2. Select board: **Arduino Mega2560**
3. Select correct serial port
4. Open `Arduino/Arduino2560control.ino`
5. Click "Upload" button to upload program to Arduino

### 2. Upload ESP32 Program

1. Install ESP32 board support in Arduino IDE
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Board Manager → Search "ESP32" and install

2. Select board: **ESP32 Dev Module**

3. Open `ESP32/ESP32_WiFi_Server.ino`

4. Configure WiFi (optional, default already configured)
   ```cpp
   const char* ssid = "RobotControl";        // WiFi hotspot name
   const char* password = "12345678";        // WiFi password
   ```

5. Click "Upload" button to upload program to ESP32

### 3. Connect WiFi and Access

1. **Connect WiFi Hotspot**
   - Search for WiFi network on PC/phone/tablet
   - Connect to WiFi hotspot named `RobotControl`
   - Enter password: `12345678`

2. **Access Web Interface**
   - Open browser
   - Visit `http://192.168.4.1`
   - Page will display robot action sequence designer

3. **Connect ESP32**
   - Click "📡 Connect ESP32" button on the page
   - System will automatically detect ESP32 connection status
   - After successful connection, status display will change to "ESP32 Connected"

## 🎮 Usage

### Basic Operation Flow

1. **Select Structure**
   - Click structure buttons on the left (RF, LF, RR, LR, TA, RE, LE, HD)

2. **Set Parameters**
   - **Forward Angle (F)**: -90 to +90 degrees
   - **Bend Angle (R)**: -90 to +90 degrees
   - **Speed Level (S)**: 1-5
   - **Pre-execution Delay**: 0-10000ms

3. **Add Action**
   - Click "➕ Add Action" button
   - Action will be automatically added to timeline

4. **Edit Sequence**
   - Drag action blocks to adjust time
   - Double-click action blocks to edit parameters

5. **Execute Control**
   - **▶️ Play Sequence**: Start executing action sequence
   - **⏸️ Pause**: Pause current execution
   - **⏹️ Stop**: Stop execution and reset
   - **🔄 Reset**: All servos return to center position

## 📝 Command Format

### New Format (Recommended)
```
Structure FAngle RAngle SSpeed DDelay
```

**Examples:**
```
RF F30 R20 S3        # Right Front: Lift 30°, Bend 20°, Speed 3
LF F-30 R15 S2       # Left Front: Drop 30°, Bend 15°, Speed 2
TA F45 R0 S4         # Tail: Clockwise swing 45°, Speed 4
RE F20 R-10 S3       # Right Ear: Rotate 20°, Pitch -10°, Speed 3
HD F0 R15 S2         # Head: Left/Right 0°, Nod 15°, Speed 2
```

### Multi-Action Sequence
```
RF F30 R20 S3;LF F-30 R15 S2;TA F45 R0 S4
```

### Special Commands
- `q` - Reset all servos to center position
- `s` - View system status

## 🔧 Core Features

### ESP32 Features
- **WiFi Hotspot**: Creates WiFi hotspot for device connection
- **Web Server**: Provides web interface and API endpoints
- **Command Forwarding**: Forwards HTTP commands to Arduino serial port
- **Response Monitoring**: Monitors Arduino responses and returns to client

### Arduino Program Features
- **WiFi Communication Support**: Communicates with ESP32 via Serial1 (baud rate 115200)
- **8 Structure Control**: Supports independent control of 8 structures
- **Action Queue Management**: Up to 10 actions queued per structure
- **Smooth Motion**: Advanced easing functions for natural motion effects
- **Concurrent Execution**: Multiple structures execute actions simultaneously

### Frontend Interface Features
- **Visual Editing**: Drag-and-drop timeline editor
- **Action Preview**: Real-time 3D robot model preview
- **WiFi Connection Management**: Automatically detects and connects to ESP32
- **Real-time Communication**: Sends commands and receives responses via WiFi in real-time
- **Status Monitoring**: Real-time display of connection status and execution progress

## 🔧 Troubleshooting

### Cannot Connect to WiFi
1. **Check ESP32**: Confirm ESP32 started normally, check serial monitor
2. **Check WiFi Name**: Confirm WiFi hotspot name is `RobotControl`
3. **Check Password**: Confirm password is `12345678`
4. **Restart ESP32**: Disconnect power and reconnect

### Cannot Access Web Page
1. **Check WiFi Connection**: Confirm connected to ESP32 WiFi hotspot
2. **Check IP Address**: Confirm IP address is correct (usually `192.168.4.1`)
   - Check ESP32 serial monitor to confirm actual IP address
   - See [Docs/IP_Address_Guide.md](Docs/IP_Address_Guide.md) for details
3. **Try Refresh**: Press F5 to refresh page
4. **Check Firewall**: Confirm firewall is not blocking connection

### Command Send Failed
1. **Check WiFi Connection**: Confirm device connected to ESP32 hotspot
2. **Check ESP32 to Arduino Connection**: Confirm serial connection is correct
3. **Check Serial Configuration**: Confirm baud rate is 115200
4. **Check ESP32 Serial Output**: Check for error messages

### Connection Status Abnormal
1. **Reconnect**: Click disconnect then reconnect
2. **Check ESP32 Running Status**: Check serial monitor
3. **Restart ESP32**: Disconnect power and reconnect

### Cross-Origin Resource Sharing (CORS) Issues
If encountering "Failed to fetch" or CORS errors:
1. Ensure ESP32 code includes CORS header support (code already includes)
2. Re-upload ESP32 firmware
3. Check browser console for CORS-related errors

## 📁 File Description

- `ESP32/ESP32_WiFi_Server.ino` - ESP32 firmware program
- `Arduino/Arduino2560control.ino` - Arduino Mega2560 program (WiFi version)
- `Web/index.html` - ESP32 server web interface
- `Web/assets/` - Resource files (SVG icons)
- `Docs/` - Detailed documentation directory

## ⚠️ Notes

1. **IP Address**
   - ESP32 IP address is usually `192.168.4.1` (default value)
   - Recommended to confirm actual IP address via serial monitor
   - See [Docs/IP_Address_Guide.md](Docs/IP_Address_Guide.md) for details

2. **WiFi Configuration**
   - Default WiFi hotspot name: `RobotControl`
   - Default password: `12345678`
   - Can be modified in `ESP32_WiFi_Server.ino`

3. **Serial Configuration**
   - ESP32 to Arduino communication baud rate: 115200
   - Ensure ESP32 and Arduino serial configurations are consistent

4. **Power Requirements**
   - 16 servos working simultaneously require sufficient current (recommended ≥5A)
   - ESP32 can be powered via Arduino's 5V or independently

5. **Multi-Device Connection**
   - Multiple devices can connect to ESP32 hotspot simultaneously
   - All devices can access and control the robot

## 📚 Related Documentation

- [Main Project README](../README.md) - Project overview and version comparison
- [PC Version](../PC_Version/README.md) - PC version documentation
- [Docs/README_ESP32_WiFi.md](Docs/README_ESP32_WiFi.md) - ESP32 WiFi detailed documentation
- [Docs/README_PC_Version.md](Docs/README_PC_Version.md) - PC version detailed documentation
- [Docs/README_WiFi.md](Docs/README_WiFi.md) - WiFi control system documentation
- [Docs/IP_Address_Guide.md](Docs/IP_Address_Guide.md) - IP address configuration guide

---

**Last Updated**: 2024

