# MojiKit Action Sequence Designer - PC Version

## 📋 Version Description

**PC Version** connects directly to Arduino Mega2560 via USB cable, using Web Serial API to enable direct communication between the browser and Arduino. Suitable for development, debugging, and close-range control scenarios.

## 🔧 Hardware Requirements

### Control Board
- **Arduino Mega2560** × 1

### Servo System
- **Standard PWM Servos** (such as SG90, MG996R, etc.) × 16
- **Servo Power Supply**: External 5V power supply, current capacity ≥5A

### Connection Cables
- USB cable (Arduino Mega2560 to computer)
- Servo connection cables (16 wires)

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

### Connection Steps

1. **Servo Connection**
   - Connect 16 servos to Arduino Mega2560 according to the pin assignment table
   - Connect servo signal wires (orange/yellow) to corresponding digital pins
   - Connect servo power positive (red) to external 5V power supply positive
   - Connect servo power negative (brown/black) to external 5V power supply negative and Arduino GND

2. **USB Connection**
   - Connect Arduino Mega2560 to computer USB port using USB cable

3. **Power Connection**
   - Provide external 5V power supply for servos (recommended: 5V/5A or higher power adapter)
   - Ensure Arduino Mega2560 is powered via USB or external power supply

## 🚀 Software Setup

### 1. Upload Arduino Program

1. Open Arduino IDE
2. Select board: **Arduino Mega2560**
3. Select correct serial port
4. Open `Arduino/USBcontrol0809.ino`
5. Click "Upload" button to upload program to Arduino

### 2. Run Frontend Interface

1. Use **Chrome** or **Edge** browser (supports Web Serial API)
2. Open `index.html` file
3. Click "🔌 Connect Arduino" button
4. Select Arduino Mega2560 in device selection dialog
5. After successful connection, status display will change to "Connected"

## 🎮 Usage

### Basic Operation Flow

1. **Select Structure**
   - Click structure buttons on the left (RF, LF, RR, LR, TA, RE, LE, HD)
   - Selected structure will be highlighted

2. **Set Parameters**
   - **Forward Angle (F)**: -90 to +90 degrees
     - Legs: Positive = lift, Negative = drop
     - Tail: Positive = clockwise swing, Negative = counterclockwise swing
     - Ears/Head: Rotation angle
   - **Bend Angle (R)**: -90 to +90 degrees
     - Legs: Bend angle
     - Tail: Up/down bend angle
     - Ears/Head: Pitch angle
   - **Speed Level (S)**: 1-5 (1 slowest, 5 fastest)
   - **Pre-execution Delay**: 0-10000ms

3. **Add Action**
   - Click "➕ Add Action" button
   - Action will be automatically added to timeline

4. **Edit Sequence**
   - Drag action blocks to adjust start time
   - Drag right edge to adjust duration
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

### Action with Delay
```
RF F30 R20 S3 D1000;LF F-30 R15 S2
```

### Special Commands
- `q` - Reset all servos to center position
- `s` - View system status

## 🔧 Core Features

### Arduino Program Features
- **8 Structure Control**: Supports independent control of 8 structures
- **Action Queue Management**: Up to 10 actions queued per structure
- **Smooth Motion**: Advanced easing functions for natural motion effects
- **Concurrent Execution**: Multiple structures execute actions simultaneously without interference
- **Delay Support**: Pre-execution delay time can be set

### Frontend Interface Features
- **Visual Editing**: Drag-and-drop timeline editor
- **Action Preview**: Real-time 3D robot model preview
- **Code Generation**: Automatically generates Arduino executable code
- **Real-time Communication**: Web Serial API directly connects to Arduino
- **Status Monitoring**: Real-time display of connection status and execution progress

## 🔧 Troubleshooting

### Cannot Connect to Arduino
1. **Check Browser**: Ensure using Chrome or Edge browser (supports Web Serial API)
2. **Check USB Connection**: Confirm USB cable connection is normal
3. **Check Serial Port**: Confirm Arduino program uploaded, serial port not occupied by other programs
4. **Check Driver**: Confirm Arduino driver installed correctly

### Abnormal Action Execution
1. **Check Servo Connections**: Confirm all servos connected correctly
2. **Check Pin Configuration**: Confirm pins match code configuration
3. **Check Power Supply**: Confirm servo power sufficient (recommended ≥5A)
4. **Check Servo Model**: Confirm using standard PWM servos

### Interface Display Issues
1. **Refresh Page**: Press F5 to refresh page
2. **Clear Cache**: Clear browser cache
3. **Check Console**: Open browser developer tools to view error messages

### Debug Mode

Enable debug mode to view detailed information:

```javascript
// Enable debug in browser console
localStorage.setItem('debug', 'true');
location.reload();
```

## 📁 File Description

- `index.html` - Frontend interface file
- `script.js` - JavaScript logic code
- `Arduino/USBcontrol0809.ino` - Arduino Mega2560 main program

## ⚠️ Notes

1. **Power Requirements**
   - 16 servos working simultaneously require sufficient current (recommended ≥5A)
   - Use external power supply for servos, do not rely solely on Arduino's 5V output

2. **Browser Compatibility**
   - Must use browser that supports Web Serial API
   - Recommended: Chrome 89+, Edge 89+

3. **Security Notice**
   - Web Serial API requires user authorization to access serial port
   - First connection requires device selection and authorization

4. **Performance Recommendations**
   - Avoid executing too many actions simultaneously
   - Set speed levels reasonably, too fast may cause servo jitter

## 📚 Related Documentation

- [Main Project README](../README.md) - Project overview and version comparison
- [ESP32 WiFi Version](../ESP32_WiFi_Version/README.md) - WiFi version documentation

---

**Last Updated**: 2024

