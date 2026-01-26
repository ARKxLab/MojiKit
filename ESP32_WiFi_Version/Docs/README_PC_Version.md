# MojiKit动作序列设计器 - PC端版本

## 概述

这是一个在PC上运行的机器人动作序列设计器，通过WiFi连接ESP32，将生成的指令转发给Arduino Mega2560舵机控制系统。

## 系统架构

```
PC (网页) ←→ WiFi ←→ ESP32 ←→ 串口 ←→ Arduino Mega2560 ←→ 舵机
```

## 使用方法

### 1. 硬件连接

- ESP32 GPIO16 → Arduino Mega2560 RX1 (19)
- ESP32 GPIO17 → Arduino Mega2560 TX1 (18)  
- ESP32 VIN → Arduino Mega2560 5V
- ESP32 GND → Arduino Mega2560 GND

### 2. 启动ESP32

1. 将`ESP32/ESP32_WiFi_Server.ino`上传到ESP32
2. ESP32将创建WiFi热点：
   - 网络名称: `RobotControl`
   - 密码: `12345678`
   - IP地址: `192.168.4.1`

### 3. 连接WiFi

1. 在PC上连接到ESP32的WiFi热点 `RobotControl`
2. 输入密码 `12345678`

### 4. 运行网页

1. 在PC上连接到ESP32的WiFi热点后，在浏览器中访问 `http://192.168.4.1`
2. 或者打开 `Web/index.html` 文件（如果已下载到本地）
3. 点击"🧪 测试连接"按钮验证连接
4. 点击"📡 连接ESP32"按钮建立连接

**或者直接访问ESP32状态页面：**
- 在浏览器中访问 `http://192.168.4.1` 查看ESP32运行状态

### 5. 设计动作序列

1. 选择机器人结构（RF、LF、RR、LR、TA、RE、LE、HD）
2. 设置角度参数和速度
3. 添加动作到时间轴
4. 播放或导出指令代码

## 功能特性

- 🎯 可视化动作序列设计
- ⏰ 时间轴编辑器
- 🤖 机器人3D预览
- 📡 WiFi实时控制
- 💾 指令代码导出
- 🔄 实时状态监控

## 指令格式

```
结构 F角度 R角度 S速度 D延迟
```

示例：
- `RF F30 R20 S2` - 右前脚，前方30度，弯曲20度，速度等级2
- `LF F-30 R15 S2 D500` - 左前脚，前方-30度，弯曲15度，速度等级2，延迟500ms

## 故障排除

### 连接问题
1. 确保已连接到ESP32的WiFi热点
2. 检查ESP32是否正常运行
3. 使用"测试连接"按钮验证连接

### 指令发送失败
1. 检查WiFi连接状态
2. 确认ESP32与Arduino的串口连接
3. 查看浏览器控制台错误信息

### 跨域访问问题（CORS）
如果遇到"Failed to fetch"或CORS错误：
1. 确保ESP32代码包含CORS头支持
2. 重新上传ESP32固件
3. 检查浏览器控制台是否有CORS相关错误
4. 某些浏览器可能需要禁用安全限制（仅用于开发测试）

## 文件说明

- `ESP32/ESP32_WiFi_Server.ino` - ESP32固件，作为WiFi热点和指令转发器
- `Arduino/Arduino2560control.ino` - Arduino Mega2560程序（WiFi版本）
- `Web/index.html` - ESP32服务器网页界面，动作序列设计器
- `README_PC_Version.md` - 本说明文档

## 注意事项

- 确保PC和ESP32在同一个WiFi网络中
- ESP32的IP地址固定为192.168.4.1
- 网页需要在支持现代浏览器的环境中运行
- 建议使用Chrome、Firefox或Edge浏览器
