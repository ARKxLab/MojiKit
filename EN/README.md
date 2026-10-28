# MojiKit Robot Action Sequence Designer

## 🎯 Project Overview

MojiKit Robot Action Sequence Designer is a visual robot action sequence design tool that supports multi-structure concurrent control of Arduino servo systems. It provides two versions: PC Version (USB direct connection) and ESP32 WiFi Version (wireless control).

## 📦 Version Description

This project provides two versions that can be selected based on usage scenarios:

### 1. PC Version (PC_Version)
- **Connection Method**: Direct connection to Arduino via USB cable
- **Communication Method**: Web Serial API (browser directly communicates with Arduino)
- **Advantages**: Simple connection, no additional hardware required, low latency
- **Use Cases**: Development and debugging, close-range control
- **Detailed Documentation**: See [PC_Version/README.md](PC_Version/README.md)

### 2. ESP32 WiFi Version (ESP32_WiFi_Version)
- **Connection Method**: Wireless connection via WiFi
- **Communication Method**: ESP32 acts as WiFi hotspot and command forwarder
- **Advantages**: Wireless control, remote operation, supports multiple devices simultaneously
- **Use Cases**: Remote control, demonstrations, multi-device control
- **Detailed Documentation**: See [ESP32_WiFi_Version/README.md](ESP32_WiFi_Version/README.md)

## 🔧 Hardware Configuration

### Control Board
- **Main Controller**: Arduino Mega2560
- **Communication Module** (WiFi version only): ESP32 development board

### Servo System
- **Servo Model**: Standard PWM servos (such as SG90, MG996R, etc., compatible with Arduino Servo library)
- **Servo Quantity**: 16 servos (8 structures × 2 servos/structure)
- **Control Method**: PWM signal control
- **Operating Voltage**: 5V (requires external power supply)

### Servo Assembly Numbering and Corresponding Relationships

| Structure Code | Structure Name | Servo Number | Arduino Pin | Function Description |
|---------------|---------------|--------------|-------------|---------------------|
| **RF** | Right Front Leg | Servo 1, 2 | D30, D31 | Lift/drop and bend control of right front leg |
| **LF** | Left Front Leg | Servo 3, 4 | D32, D33 | Lift/drop and bend control of left front leg |
| **RR** | Right Rear Leg | Servo 5, 6 | D34, D35 | Lift/drop and bend control of right rear leg |
| **LR** | Left Rear Leg | Servo 7, 8 | D36, D37 | Lift/drop and bend control of left rear leg |
| **TA** | Tail | Servo 9, 10 | D38, D39 | Left/right swing and up/down bend control of tail |
| **RE** | Right Ear | Servo 11, 12 | D40, D41 | Rotation and pitch control of right ear |
| **LE** | Left Ear | Servo 13, 14 | D42, D43 | Rotation and pitch control of left ear |
| **HD** | Head | Servo 15, 16 | D44, D45 | Left/right rotation and up/down nod control of head |

### Pin Connection Table

```
Arduino Mega2560 Pin Assignment:
┌─────────┬──────────┬──────────────────┐
│ Pin     │ Servo No.│ Structure Part   │
├─────────┼──────────┼──────────────────┤
│ D30     │ Servo 1  │ Right Front-S1   │
│ D31     │ Servo 2  │ Right Front-S2   │
│ D32     │ Servo 3  │ Left Front-S1    │
│ D33     │ Servo 4  │ Left Front-S2    │
│ D34     │ Servo 5  │ Right Rear-S1    │
│ D35     │ Servo 6  │ Right Rear-S2    │
│ D36     │ Servo 7  │ Left Rear-S1     │
│ D37     │ Servo 8  │ Left Rear-S2     │
│ D38     │ Servo 9  │ Tail-S1          │
│ D39     │ Servo 10 │ Tail-S2          │
│ D40     │ Servo 11 │ Right Ear-S1     │
│ D41     │ Servo 12 │ Right Ear-S2     │
│ D42     │ Servo 13 │ Left Ear-S1      │
│ D43     │ Servo 14 │ Left Ear-S2      │
│ D44     │ Servo 15 │ Head-S1          │
│ D45     │ Servo 16 │ Head-S2          │
└─────────┴──────────┴──────────────────┘
```

### Mechanical Structure Design Reference

The **RR (right rear leg), RF (right front leg), LR (left rear leg), LF (left front leg), and TA (tail)** structures in this project use the finger structure design from the open-source [AmazingHand](https://github.com/pollen-robotics/AmazingHand) project.

**Acknowledgments**: Thanks to the Pollen Robotics team for developing the AmazingHand project, which is licensed under Apache-2.0, with mechanical design licensed under CC BY 4.0.

### Power Requirements
- **Arduino Mega2560**: Powered via USB or external power supply (7-12V recommended)
- **Servo Power**: Requires external 5V power supply, current capacity recommended ≥5A (for 16 servos working simultaneously)
- **ESP32** (WiFi version only): Can be powered via Arduino's 5V or independently

## 🚀 Quick Start

### Choose Version
1. **PC Version**: Suitable for development and debugging, simple connection
   - See [PC_Version/README.md](PC_Version/README.md)
   
2. **ESP32 WiFi Version**: Suitable for remote control and demonstrations
   - See [ESP32_WiFi_Version/README.md](ESP32_WiFi_Version/README.md)

### Basic Usage Flow
1. **Hardware Connection**: Connect servos according to the pin connection table
2. **Upload Program**: Upload the Arduino program of the corresponding version to Arduino Mega2560
3. **Run Interface**: Open the web interface (PC version opens directly, WiFi version accesses after connecting to WiFi)
4. **Design Actions**: Use the visual interface to design action sequences
5. **Execute Control**: Play, pause, stop action sequences

## 📁 Project Structure

```
MojiKit_pet_robot_behavior_design/
├── README.md                          # This file (project overview)
├── .gitignore                         # Git ignore file
│
├── PC_Version/                         # PC Version
│   ├── README.md                      # PC version detailed documentation
│   ├── index.html                     # Frontend interface
│   ├── script.js                      # JavaScript logic
│   └── Arduino/
│       └── USBcontrol0809.ino         # Arduino main program
│
└── ESP32_WiFi_Version/                # ESP32 WiFi Version
    ├── README.md                      # WiFi version detailed documentation
    ├── ESP32/
    │   └── ESP32_WiFi_Server.ino      # ESP32 firmware
    ├── Arduino/
    │   └── Arduino2560control.ino     # Arduino program (WiFi version)
    ├── Web/
    │   ├── index.html                 # ESP32 server web page
    │   └── assets/                    # Resource files (SVG icons)
    └── Docs/
        ├── README_ESP32_WiFi.md       # ESP32 WiFi detailed documentation
        ├── README_PC_Version.md       # PC version detailed documentation
        ├── README_WiFi.md             # WiFi control system documentation
        └── IP_Address_Guide.md        # IP address configuration guide
```

## 🎮 Core Features

### Action Control
- **8 Structure Control**: RF (Right Front), LF (Left Front), RR (Right Rear), LR (Left Rear), TA (Tail), RE (Right Ear), LE (Left Ear), HD (Head)
- **Action Queue Management**: Up to 10 actions queued per structure
- **Smooth Motion**: Advanced easing functions for natural motion effects
- **Concurrent Execution**: Multiple structures execute actions simultaneously without interference
- **Delay Support**: Pre-execution delay time can be set

### Visual Interface
- **Timeline Editor**: Drag-and-drop timeline for intuitive action sequence editing
- **3D Preview**: Real-time robot SVG model preview
- **Code Generation**: Automatically generates Arduino executable code
- **Real-time Monitoring**: Connection status, execution progress, error prompts

## 📝 Command Format

### New Format (Recommended)
```
Structure FAngle RAngle SSpeed DDelay
```

**Examples:**
- `RF F30 R20 S3` - Right Front: Lift 30°, Bend 20°, Speed 3
- `LF F-30 R15 S2 D500` - Left Front: Drop 30°, Bend 15°, Speed 2, Delay 500ms
- `TA F45 R0 S4` - Tail: Clockwise swing 45°, Speed 4
- `RE F20 R-10 S3` - Right Ear: Rotate 20°, Pitch -10°, Speed 3
- `HD F0 R15 S2` - Head: Left/Right 0°, Nod 15°, Speed 2

### Multi-Action Sequence
```
RF F30 R20 S3;LF F-30 R15 S2;TA F45 R0 S4
```

### Special Commands
- `q` - Reset all servos to center position
- `s` - View system status
- `duration(command)` - Calculate command execution time

## 🔧 Troubleshooting

### Common Issues

1. **Cannot Connect to Arduino/ESP32**
   - PC Version: Ensure using Chrome or Edge browser, check USB cable connection
   - WiFi Version: Confirm connected to ESP32 WiFi hotspot, check IP address

2. **Abnormal Action Execution**
   - Check if servo connections are correct
   - Confirm pin configuration matches the connection table
   - Check if power supply is sufficient

3. **Servos Not Moving**
   - Check servo power connections
   - Confirm servo model compatibility (standard PWM servos)
   - Check if Arduino program uploaded successfully

For detailed troubleshooting, please refer to the README documentation of the corresponding version.

## 📄 License

This project is licensed under the MIT License. See LICENSE file for details.

## 🤝 Contributing

Contributions of code, bug reports, or suggestions are welcome!

1. Fork the project
2. Create a feature branch
3. Commit your changes
4. Submit a Pull Request

## 📞 Contact Us

For questions or suggestions, please contact us through:
- Project Issues: [GitHub Issues](https://github.com/your-repo/issues)
- Email: helw24@mails.tsinghua.edu.cn / buaaheliwen@163.com
---

**Note**: Please ensure hardware connections are correct before use. It is recommended to operate under professional guidance. Servos require sufficient power supply. It is recommended to use an external power adapter.

