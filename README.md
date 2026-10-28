# MojiKit 机器人动作序列设计器 / MojiKit Robot Action Sequence Designer

**中文** | [English](EN/README.md)

## 🎯 项目概述 / Project Overview

MojiKit 机器人动作序列设计器是一个可视化机器人动作序列设计工具，支持多结构并发控制的Arduino舵机系统。提供两种版本：PC端版本（USB直连）和ESP32 WiFi版本（无线控制）。

MojiKit Robot Action Sequence Designer is a visual robot action sequence design tool that supports multi-structure concurrent control of Arduino servo systems. It provides two versions: PC Version (USB direct connection) and ESP32 WiFi Version (wireless control).

## 📦 版本说明 / Version Description

本项目提供两个版本，可根据使用场景选择：
This project provides two versions that can be selected based on usage scenarios:

### 1. PC端版本（PC_Version）
- **连接方式**：通过USB线直接连接Arduino
- **通信方式**：Web Serial API（浏览器直接与Arduino通信）
- **优点**：连接简单，无需额外硬件，延迟低
- **适用场景**：开发调试、近距离控制
- **详细说明**：请查看 [PC_Version/README.md](PC_Version/README.md) | [English](EN/PC_Version/README.md)

### 2. ESP32 WiFi版本（ESP32_WiFi_Version）
- **连接方式**：通过WiFi无线连接
- **通信方式**：ESP32作为WiFi热点和指令转发器
- **优点**：无线控制，可远程操作，支持多设备同时连接
- **适用场景**：远程控制、演示展示、多设备控制
- **详细说明**：请查看 [ESP32_WiFi_Version/README.md](ESP32_WiFi_Version/README.md) | [English](EN/ESP32_WiFi_Version/README.md)

## 🔧 硬件配置 / Hardware Configuration

### 控制板 / Control Board
- **主控制器 / Main Controller**：Arduino Mega2560
- **通信模块 / Communication Module**（仅WiFi版本 / WiFi version only）：ESP32开发板 / ESP32 Development Board

### 舵机系统 / Servo System
- **舵机型号 / Servo Model**：标准PWM舵机（如SG90、MG996R等，支持Arduino Servo库）/ Standard PWM servos (such as SG90, MG996R, etc., compatible with Arduino Servo library)
- **舵机数量 / Servo Quantity**：16个舵机（8个结构 × 2个舵机/结构）/ 16 servos (8 structures × 2 servos/structure)
- **控制方式 / Control Method**：PWM信号控制 / PWM signal control
- **工作电压 / Operating Voltage**：5V（需外部电源供电）/ 5V (requires external power supply)

### 舵机组装编号与对应关系

| 结构代码 | 结构名称 | 舵机编号 | Arduino引脚 | 功能说明 |
|---------|---------|---------|------------|---------|
| **RF** | 右前脚 | 舵机1, 2 | D30, D31 | 右前脚的抬起/放下和弯曲控制 |
| **LF** | 左前脚 | 舵机3, 4 | D32, D33 | 左前脚的抬起/放下和弯曲控制 |
| **RR** | 右后脚 | 舵机5, 6 | D34, D35 | 右后脚的抬起/放下和弯曲控制 |
| **LR** | 左后脚 | 舵机7, 8 | D36, D37 | 左后脚的抬起/放下和弯曲控制 |
| **TA** | 尾巴 | 舵机9, 10 | D38, D39 | 尾巴的左右摇摆和上下弯曲控制 |
| **RE** | 右耳 | 舵机11, 12 | D40, D41 | 右耳的旋转和俯仰控制 |
| **LE** | 左耳 | 舵机13, 14 | D42, D43 | 左耳的旋转和俯仰控制 |
| **HD** | 头部 | 舵机15, 16 | D44, D45 | 头部的左右转动和上下点头控制 |

### 引脚连接表

```
Arduino Mega2560引脚分配：
┌─────────┬──────────┬──────────────┐
│ 引脚    │ 舵机编号 │ 结构部位     │
├─────────┼──────────┼──────────────┤
│ D30     │ 舵机1    │ 右前脚-舵机1 │
│ D31     │ 舵机2    │ 右前脚-舵机2 │
│ D32     │ 舵机3    │ 左前脚-舵机1 │
│ D33     │ 舵机4    │ 左前脚-舵机2 │
│ D34     │ 舵机5    │ 右后脚-舵机1 │
│ D35     │ 舵机6    │ 右后脚-舵机2 │
│ D36     │ 舵机7    │ 左后脚-舵机1 │
│ D37     │ 舵机8    │ 左后脚-舵机2 │
│ D38     │ 舵机9    │ 尾巴-舵机1   │
│ D39     │ 舵机10   │ 尾巴-舵机2   │
│ D40     │ 舵机11   │ 右耳-舵机1   │
│ D41     │ 舵机12   │ 右耳-舵机2   │
│ D42     │ 舵机13   │ 左耳-舵机1   │
│ D43     │ 舵机14   │ 左耳-舵机2   │
│ D44     │ 舵机15   │ 头部-舵机1   │
│ D45     │ 舵机16   │ 头部-舵机2   │
└─────────┴──────────┴──────────────┘
```

### 机械结构设计参考 / Mechanical Structure Design Reference

本项目中的 **RR（右后脚）、RF（右前脚）、LR（左后脚）、LF（左前脚）、TA（尾巴）** 结构使用了 [AmazingHand](https://github.com/pollen-robotics/AmazingHand) 开源项目中的手指部分结构设计。

The **RR (right rear leg), RF (right front leg), LR (left rear leg), LF (left front leg), and TA (tail)** structures in this project use the finger structure design from the open-source [AmazingHand](https://github.com/pollen-robotics/AmazingHand) project.

**致谢 / Acknowledgments**：感谢 Pollen Robotics 团队开发的 AmazingHand 项目，该项目采用 Apache-2.0 许可证，机械设计采用 CC BY 4.0 许可证。

**Acknowledgments**: Thanks to the Pollen Robotics team for developing the AmazingHand project, which is licensed under Apache-2.0, with mechanical design licensed under CC BY 4.0.

### 电源要求 / Power Requirements
- **Arduino Mega2560**：通过USB或外部电源供电（7-12V推荐）/ Powered via USB or external power supply (7-12V recommended)
- **舵机电源 / Servo Power**：需要外部5V电源，电流容量建议≥5A（16个舵机同时工作）/ Requires external 5V power supply, current capacity recommended ≥5A (for 16 servos working simultaneously)
- **ESP32**（仅WiFi版本 / WiFi version only）：可通过Arduino的5V供电或独立供电 / Can be powered via Arduino's 5V or independently

## 🚀 快速开始 / Quick Start

### 选择版本 / Choose Version
1. **PC端版本 / PC Version**：适合开发调试，连接简单 / Suitable for development and debugging, simple connection
   - 查看 / See [PC_Version/README.md](PC_Version/README.md) | [English](EN/PC_Version/README.md)
   
2. **ESP32 WiFi版本 / ESP32 WiFi Version**：适合远程控制和演示 / Suitable for remote control and demonstrations
   - 查看 / See [ESP32_WiFi_Version/README.md](ESP32_WiFi_Version/README.md) | [English](EN/ESP32_WiFi_Version/README.md)

### 基本使用流程 / Basic Usage Flow
1. **硬件连接 / Hardware Connection**：按照引脚连接表连接舵机 / Connect servos according to pin connection table
2. **上传程序 / Upload Program**：将对应版本的Arduino程序上传到Arduino Mega2560 / Upload Arduino program of corresponding version to Arduino Mega2560
3. **运行界面 / Run Interface**：打开网页界面（PC版本直接打开，WiFi版本连接WiFi后访问）/ Open web interface (PC version opens directly, WiFi version accesses after connecting to WiFi)
4. **设计动作 / Design Actions**：使用可视化界面设计动作序列 / Use visual interface to design action sequences
5. **执行控制 / Execute Control**：播放、暂停、停止动作序列 / Play, pause, stop action sequences

## 📁 项目结构 / Project Structure

```
MojiKit_pet_robot_behavior_design/
├── README.md                          # 本文件（项目总览）/ This file (project overview)
├── .gitignore                         # Git忽略文件 / Git ignore file
│
├── PC_Version/                        # PC端版本 / PC Version
│   ├── README.md                      # PC版本详细说明 / PC version documentation
│   ├── index.html                      # 前端界面 / Frontend interface
│   ├── script.js                       # JavaScript逻辑 / JavaScript logic
│   └── Arduino/
│       └── USBcontrol0809.ino         # Arduino主程序 / Arduino main program
│
├── ESP32_WiFi_Version/                # ESP32 WiFi版本 / ESP32 WiFi Version
│   ├── README.md                      # WiFi版本详细说明 / WiFi version documentation
│   ├── ESP32/
│   │   └── ESP32_WiFi_Server.ino      # ESP32固件 / ESP32 firmware
│   ├── Arduino/
│   │   └── Arduino2560control.ino     # Arduino程序（WiFi版本）/ Arduino program (WiFi version)
│   ├── Web/
│   │   ├── index.html                 # ESP32服务器网页 / ESP32 server web page
│   │   └── assets/                    # 资源文件（SVG图标）/ Resource files (SVG icons)
│   └── Docs/
│       ├── README_ESP32_WiFi.md       # ESP32 WiFi详细文档 / ESP32 WiFi detailed docs
│       ├── README_PC_Version.md        # PC版本详细文档 / PC version detailed docs
│       ├── README_WiFi.md              # WiFi控制系统说明 / WiFi control system docs
│       └── IP地址说明.md               # IP地址配置说明 / IP address configuration guide
│
└── EN/                                # 英文版文档 / English Documentation
    ├── README.md                      # 英文版项目总览 / English project overview
    ├── PC_Version/                    # PC版本英文文档 / PC version English docs
    └── ESP32_WiFi_Version/            # ESP32 WiFi版本英文文档 / ESP32 WiFi version English docs
```

## 🎮 核心功能

### 动作控制
- **8个结构控制**：RF(右前脚)、LF(左前脚)、RR(右后脚)、LR(左后脚)、TA(尾巴)、RE(右耳)、LE(左耳)、HD(头部)
- **动作队列管理**：每个结构最多10个动作排队执行
- **平滑运动**：高级缓动函数，实现自然运动效果
- **并发执行**：多个结构同时执行动作，互不干扰
- **延迟支持**：动作执行前可设置延迟时间

### 可视化界面
- **时间轴编辑器**：拖拽式时间轴，直观编辑动作序列
- **3D预览**：机器人SVG模型实时预览
- **代码生成**：自动生成Arduino可执行的代码
- **实时监控**：连接状态、执行进度、错误提示

## 📝 指令格式

### 新格式（推荐）
```
结构 F角度 R角度 S速度 D延迟
```

**示例：**
- `RF F30 R20 S3` - 右前脚：抬起30度，弯曲20度，速度3
- `LF F-30 R15 S2 D500` - 左前脚：放下30度，弯曲15度，速度2，延迟500ms
- `TA F45 R0 S4` - 尾巴：顺时针摇摆45度，速度4
- `RE F20 R-10 S3` - 右耳：旋转20度，俯仰-10度，速度3
- `HD F0 R15 S2` - 头部：左右0度，上下点头15度，速度2

### 多动作序列
```
RF F30 R20 S3;LF F-30 R15 S2;TA F45 R0 S4
```

### 特殊指令
- `q` - 复位所有舵机到中位
- `s` - 查看系统状态
- `duration(指令)` - 计算指令执行时间

## 🔧 故障排除

### 常见问题

1. **无法连接Arduino/ESP32**
   - PC版本：确保使用Chrome或Edge浏览器，检查USB线连接
   - WiFi版本：确认已连接到ESP32的WiFi热点，检查IP地址

2. **动作执行异常**
   - 检查舵机连接是否正确
   - 确认引脚配置与连接表一致
   - 检查电源供应是否充足

3. **舵机不动作**
   - 检查舵机电源连接
   - 确认舵机型号兼容（标准PWM舵机）
   - 检查Arduino程序是否正常上传

详细故障排除请查看对应版本的README文档。

## 📄 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！

1. Fork项目
2. 创建功能分支
3. 提交更改
4. 发起Pull Request

## 📞 联系我们 / Contact Us

如有问题或建议，请通过以下方式联系：
For questions or suggestions, please contact us through:

- 项目Issues / Project Issues：[GitHub Issues](https://github.com/your-repo/issues)
- 邮箱 / Email：helw24@mails.tsinghua.edu.cn / buaaheliwen@163.com

---

**注意 / Note**：使用前请确保硬件连接正确，建议在专业人士指导下操作。舵机需要足够的电源供应，建议使用外部电源适配器。

Please ensure hardware connections are correct before use. It is recommended to operate under professional guidance. Servos require sufficient power supply. It is recommended to use an external power adapter.

---

## 📚 文档语言 / Documentation Languages

- **中文文档**：根目录下的所有文档和子文件夹
- **English Documentation**: See [EN/](EN/) folder for all English documentation

**中文** | [English](EN/README.md)
