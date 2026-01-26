# WiFi Control System User Guide

## 🎯 System Overview

This system provides WiFi communication functionality for Arduino Mega2560 servo control system through ESP32 module, allowing users to remotely control the robot through web interface.

## 🔧 Hardware Connection

### Arduino Mega2560 + ESP32 Connection
```
ESP32 Module          Arduino Mega2560
TX (GPIO17)   →    RX1 (19)
RX (GPIO16)   →    TX1 (18)
VIN           →    5V
GND           →    GND
```

### Servo Connection
```
Servo 1-16  →  D30-D45 (Connect in order)
```

## 📋 File Description

1. **Arduino/Arduino2560control.ino** - Arduino Mega2560 main program (WiFi support added)
2. **ESP32/ESP32_WiFi_Server.ino** - ESP32 WiFi server program
3. **Web/index.html** - ESP32 server web interface
4. **README_WiFi.md** - This documentation

## 🚀 Usage Steps

### 1. Upload Programs

#### Arduino Mega2560 Program
1. Open Arduino IDE
2. Select board: Arduino Mega2560
3. Open `Arduino/Arduino2560control.ino` file
4. Upload program to Arduino Mega2560

#### ESP32 Program
1. Open Arduino IDE
2. Select board: ESP32 Dev Module
3. Open `ESP32/ESP32_WiFi_Server.ino` file
4. Modify WiFi configuration (optional):
   ```cpp
   const char* ssid = "RobotControl";        // WiFi hotspot name
   const char* password = "12345678";        // WiFi password
   ```
5. Upload program to ESP32

### 2. Connect Hardware

1. Connect ESP32 and Arduino Mega2560 according to hardware connection diagram
2. Connect all servos to corresponding pins
3. Power Arduino Mega2560

### 3. Start System

1. After power on, ESP32 will automatically create WiFi hotspot
2. Hotspot name: `RobotControl`
3. Password: `12345678`
4. Default IP address: `192.168.4.1`

### 4. Connect and Control

1. Connect phone or computer to WiFi hotspot `RobotControl`
2. Password: `12345678`
3. Open browser, visit: `http://192.168.4.1`
4. Start remote controlling the robot!

## 🎮 Control Interface

### Quick Commands
- **Reset** - All servos return to center position
- **Status** - View current system status
- **Right Front Leg Lift** - Right front leg lifts 30 degrees
- **Left Front Leg Drop** - Left front leg drops 30 degrees
- **Tail Swing** - Tail swings 20 degrees
- **Right Ear Rotate** - Right ear rotates 15 degrees

### Custom Commands

#### New Format Commands
```
RF F30 R20 S3    # Right front leg: lift 30°, bend 20°, speed 3
LF F-30 R15 S3   # Left front leg: drop 30°, bend 15°, speed 3
TA F20 R10 S2    # Tail: swing 20°, bend 10°, speed 2
RE F15 R5 S3     # Right ear: rotate 15°, pitch 5°, speed 3
HD F10 R-5 S3    # Head: left/right rotate 10°, nod -5°, speed 3
```

#### Old Format Commands
```
RFu40,-20,3      # Right front leg: Servo 1 rotate 40°, Servo 2 rotate -20°, speed 3
LFu-30,15,3      # Left front leg: Servo 1 rotate -30°, Servo 2 rotate 15°, speed 3
```

#### Multi-Action Commands
```
RF F30 R20;LF F-30 R15;RR F25 R-10    # Control multiple structures simultaneously
```

#### Special Commands
```
q                # Reset all servos
s                # View system status
duration(RF F40 R20 S5;LF F-30 R15 S3)  # Calculate command execution time
```

## 📊 Status Feedback

The system will return the following status information in real-time:
- Command reception confirmation
- Action execution status
- Error messages
- System status

## 🔧 Troubleshooting

### Common Issues

1. **Compilation Error: Servo Library Incompatible**
   - **Problem**: ESP32 does not support standard Servo library
   - **Solution**: ESP32 only handles WiFi communication, servo control is handled by Arduino Mega2560
   - **Note**: Do not upload Arduino code to ESP32

2. **ESP32Servo Library Error**
   - **Problem**: ESP32Servo library dependency issues
   - **Solution**: ESP32 only needs to run WiFi server program, does not need servo control
   - **Note**: ESP32 only uses `ESP32/ESP32_WiFi_Server.ino` file, does not control servos

3. **Cannot Connect to WiFi Hotspot**
   - Check if ESP32 started normally
   - Confirm WiFi hotspot name and password
   - Restart ESP32

4. **Commands Not Responding**
   - Check ESP32 and Arduino connection
   - Confirm serial baud rate setting (115200)
   - Check if Arduino program is running normally

5. **Servos Not Moving**
   - Check servo power connections
   - Confirm servo pin connections are correct
   - Check pin definitions in Arduino program

6. **Web Page Cannot Be Accessed**
   - Confirm connected to correct WiFi hotspot
   - Check if IP address is correct
   - Restart ESP32

7. **Character Encoding Error**
   - **Problem**: Invalid characters in HTML
   - **Solution**: Character issues in ESP32 code have been fixed
   - **Note**: Use latest version of `ESP32/ESP32_WiFi_Server.ino`

### Debugging Methods

1. **View Serial Output**
   - Arduino Mega2560: Connect USB to view Serial output
   - ESP32: Connect USB to view Serial output

2. **Check Network Connection**
   - Confirm WiFi hotspot has been created
   - Check if devices are connected
   - Test network connectivity

3. **Test Communication**
   - Send test commands through serial port
   - Check communication between ESP32 and Arduino
   - Verify WiFi command forwarding functionality

## 📝 Notes

1. **Power Requirements**
   - Arduino Mega2560 needs stable 5V power supply
   - Servos need sufficient current supply
   - Recommend using external power adapter

2. **Network Security**
   - Default WiFi password is simple, recommend changing
   - Only use in internal network environment
   - Do not connect to public networks

3. **System Stability**
   - Avoid frequently sending commands
   - Regularly check system status
   - Handle error messages promptly

4. **Compatibility**
   - Arduino code only applies to Arduino Mega2560
   - ESP32 code only applies to ESP32 development board
   - Do not confuse the two programs
   - ESP32 only handles WiFi communication, does not control servos

## 🔄 Update Log

- v1.0.0 - Initial version, supports basic WiFi control functionality
- v1.1.0 - Added status feedback and error handling
- v1.2.0 - Optimized user interface and response speed
- v1.3.0 - Fixed character encoding and compatibility issues
- v1.4.0 - Removed ESP32 servo control code, simplified system architecture

## 📞 Technical Support

If you have questions, please check:
1. Whether hardware connections are correct
2. Whether programs uploaded successfully
3. Whether network connection is normal
4. Whether power supply is sufficient
5. Whether correct board type is used
6. Whether ESP32 only runs WiFi server program

