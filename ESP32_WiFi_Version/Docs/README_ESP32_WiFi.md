# MojiKit 机器人动作序列设计器 - ESP32 WiFi版

## 概述

这是一个基于ESP32 WiFi的机器人动作序列设计器，允许用户通过WiFi连接设计机器人的动作序列，并通过ESP32将生成的代码通过串口发送给Arduino Mega2560。

## 硬件连接

### ESP32与Arduino Mega2560连接
- ESP32 TX (GPIO17) → Arduino Mega2560 RX1 (19)
- ESP32 RX (GPIO16) → Arduino Mega2560 TX1 (18)
- ESP32 VIN → Arduino Mega2560 5V
- ESP32 GND → Arduino Mega2560 GND

### 串口配置
- 波特率: 115200
- 数据位: 8
- 停止位: 1
- 无奇偶校验

## 使用方法

### 1. 连接WiFi
1. 在PC上搜索WiFi网络
2. 连接到名为 `RobotControl` 的WiFi热点
3. 输入密码: `12345678`

### 2. 访问网页
1. 打开浏览器
2. 访问 `http://192.168.4.1`
3. 页面将显示机器人动作序列设计器

### 3. 连接ESP32
1. 点击页面上的 "📡 连接ESP32" 按钮
2. 系统将自动检测ESP32连接状态
3. 连接成功后，状态显示将变为 "已连接ESP32"

### 4. 设计动作序列
1. 选择机器人结构（左耳、右耳、头部、脚等）
2. 设置角度参数
3. 添加动作到时间轴
4. 调整动作时序

### 5. 发送代码
1. 点击 "播放序列" 按钮开始执行
2. 系统将自动将生成的代码发送给ESP32
3. ESP32通过串口将代码转发给Arduino Mega2560
4. 在代码输出区域可以看到ESP32的响应

## 功能特性

- **WiFi连接**: 通过ESP32的WiFi热点连接，无需USB线
- **实时通信**: 支持实时发送命令和接收响应
- **状态监控**: 自动监控连接状态，断线自动重连
- **代码生成**: 自动生成Arduino代码并发送执行
- **可视化设计**: 直观的图形界面设计机器人动作

## 技术架构

```
PC浏览器 ←→ ESP32 WiFi热点 ←→ ESP32 Web服务器 ←→ ESP32串口 ←→ Arduino Mega2560
```

### ESP32功能
- 创建WiFi热点
- 提供Web服务器
- 转发HTTP命令到串口
- 监控串口响应

### 网页功能
- 机器人动作序列设计
- WiFi连接管理
- 实时命令发送
- 响应状态显示

## 故障排除

### 无法连接WiFi
- 检查ESP32是否正常启动
- 确认WiFi名称和密码正确
- 重启ESP32

### 无法访问网页
- 确认已连接到正确的WiFi网络
- 检查浏览器地址是否正确
- 尝试刷新页面

### 命令发送失败
- 检查ESP32与Arduino的连接
- 确认串口配置正确
- 查看ESP32串口输出

### 连接状态异常
- 点击断开连接后重新连接
- 检查ESP32是否正常运行
- 重启ESP32

## 开发说明

### 修改WiFi配置
在 `ESP32/ESP32_WiFi_Server.ino` 中修改：
```cpp
const char* ssid = "你的WiFi名称";
const char* password = "你的WiFi密码";
```

### 修改串口配置
在 `ESP32/ESP32_WiFi_Server.ino` 中修改：
```cpp
#define SERIAL_BAUD_RATE 你的波特率
#define ARDUINO_RX_PIN 你的RX引脚
#define ARDUINO_TX_PIN 你的TX引脚
```

### 添加新的API接口
在 `setupWebServer()` 函数中添加新的路由处理。

## 版本历史

- v1.0: 初始版本，支持基本的WiFi连接和命令转发
- 支持机器人动作序列设计
- 支持实时命令发送和响应显示

## 许可证

本项目采用MIT许可证，详见LICENSE文件。
