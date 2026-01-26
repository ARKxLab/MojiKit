// 多结构并发控制 + 多动作排队执行版本 + WiFi通信支持
// 注意：此代码仅适用于Arduino Mega2560，不适用于ESP32
#include <Servo.h>

const int NUM_STRUCTS = 8;  // 增加到8个结构（5个脚+3个头部结构）
const int MAX_QUEUE = 10;  // 增加队列容量
const int mid = 90;
int speedLevels[6] = {0, 30, 20, 10, 5, 2};
const int COMMAND_DELAY = 0; // 命令间延迟改为0ms，实现真正的并行执行

// WiFi通信配置
#define WIFI_BAUD_RATE 115200  // ESP32默认波特率
#define WIFI_TIMEOUT 1000      // WiFi通信超时时间(ms)

Servo servos[NUM_STRUCTS * 2];
int pins[NUM_STRUCTS * 2] = {30,31, 32,33, 34,35, 36,37, 38,39, 40,41, 42,43, 44,45};
int currentPos[NUM_STRUCTS * 2];

struct MotionCommand {
  int s1_idx, s2_idx;
  int start1, target1;
  int start2, target2;
  int steps;
  int currentStep;
  int delayPerStep;
  unsigned long lastMoveTime;
  bool active;  // 标记命令是否激活
  bool smoothMotion;  // 是否启用平滑运动
  int preDelay;  // 执行前延迟时间(ms)
  bool preDelaySet;  // 是否设置了执行前延迟
  bool isFirstCommand;  // 标记是否为队列中的第一个命令
};

MotionCommand motionQueue[NUM_STRUCTS][MAX_QUEUE];
int queueStart[NUM_STRUCTS] = {0};
int queueEnd[NUM_STRUCTS] = {0};
int queueSize[NUM_STRUCTS] = {0};  // 当前队列中的命令数量

// 函数声明
void calculateDuration(String input);
unsigned long parseCommandDuration(String cmd);
int getCommandGroup(String cmd);
unsigned long parseNewCommandDuration(int group, String cmd);
unsigned long parseOldCommandDuration(int group, String cmd);
void calculateServoAngles(int group, int forwardAngle, int bendAngle, int &rotation1, int &rotation2);
String getGroupName(int group);
void handleWiFiInput();
void processInput(String input, bool isWiFi = false);
void sendWiFiResponse(String message);
bool parseCommand(String input, bool isWiFi = false);
bool parseNewCommand(int group, String cmd, bool isWiFi = false);
bool enqueue(int group, int target1, int target2, int speed, int preDelay = 0);
void handleMotionQueues();
void resetAll();
void showStatus();
void handleSerial();

void setup() {
  Serial.begin(9600);
  
  // 初始化WiFi串口通信（Serial1用于ESP32通信）
  Serial1.begin(WIFI_BAUD_RATE);
  Serial1.setTimeout(WIFI_TIMEOUT);
  
  for (int i = 0; i < NUM_STRUCTS * 2; i++) {
    servos[i].attach(pins[i]);
    currentPos[i] = mid;
  }
  
  // 初始化队列
  for (int i = 0; i < NUM_STRUCTS; i++) {
    for (int j = 0; j < MAX_QUEUE; j++) {
      motionQueue[i][j].active = false;
      motionQueue[i][j].isFirstCommand = false;
    }
  }
  
  Serial.println("✅ System startup, supporting multi-structure + multi-action queued execution + smooth motion optimization + WiFi communication + parallel execution");
  Serial.println("🎯 Support command format:");
  Serial.println("  ▶ New format: RF F30 R20 S3");
  Serial.println("  ▶ Old format: RFu40,-20,3");
  Serial.println("  ▶ Multiple actions: RF F30 R20;LF F-30 R15;RR F25 R-10");
  Serial.println("  ▶ Reset: q");
  Serial.println("  ▶ View status: s");
  Serial.println("  ▶ Structure corresponding: RF(右前脚) LF(左前脚) RR(右后脚) LR(左后脚) TA(尾巴) RE(右耳) LE(左耳) HD(头部)");
  Serial.println("  ▶ New format description:");
  Serial.println("    - RF/LF: F<Lift/Drop> R<Flexion Angle> (Positive F = Lift, Negative F = Drop)");
  Serial.println("    - RR/LR: F<Lift/Drop> R<Flexion Angle> (Positive F = Lift, Negative F = Drop)");
  Serial.println("    - TA: F<Wag> R<Flexion Angle> (Positive F = Wag, Negative F = Wag)");
  Serial.println("    - RE/LE: F<Rotation Angle> R<Pitch Angle>");
  Serial.println("    - HD: F<Rotation Angle> R<Pitch Angle>");
  Serial.println("  ▶ Delay function: D<Delay ms> Wait for specified time before action execution");
  Serial.println("  ▶ Time calculation: duration(RF F40 R20 S5;RF F-40 R20 S5 D1000;RE F-40 R20 S5) Calculate instruction execution time");
  Serial.println("  ▶ Parallel execution: Different action sequences of different structures will be executed at the same time, without waiting");
  Serial.println("📶 WiFi communication enabled, can send commands through ESP32");
  
  // 发送WiFi初始化确认
  sendWiFiResponse("✅ System startup completed, WiFi communication enabled, parallel execution mode activated");
}

void loop() {
  handleSerial();
  handleWiFiInput(); // 处理WiFi输入
  handleMotionQueues();
  
  // 添加适当的延时控制，减少CPU负载和发热
  // 根据是否有活动动作调整延时
  bool hasActiveMotion = false;
  for (int i = 0; i < NUM_STRUCTS; i++) {
    if (queueSize[i] > 0) {
      hasActiveMotion = true;
      break;
    }
  }
  
  if (hasActiveMotion) {
    // 有动作时，使用较短的延时保持响应性
    delay(5); // 5ms延时，减少CPU负载
  } else {
    // 无动作时，使用较长的延时节省资源
    delay(20); // 20ms延时，大幅减少CPU负载
  }
}
// 处理串口输入
void handleSerial() {
  if (!Serial.available()) return;
  String input = Serial.readStringUntil('\n');
  input.trim(); 
  processInput(input, false);
}

// 处理WiFi输入
void handleWiFiInput() {
  if (!Serial1.available()) return;
  String input = Serial1.readStringUntil('\n');
  input.trim();
  
  // 发送WiFi指令确认
  sendWiFiResponse("📶 Received WiFi command: " + input);
  
  processInput(input, true);
}

// 发送WiFi响应
void sendWiFiResponse(String message) {
  Serial1.println(message);
  Serial1.flush(); // 确保数据发送完成
}

// 处理串口和WiFi输入
void processInput(String input, bool isWiFi = false) {
  input.replace("；", ";");

  if (input.equalsIgnoreCase("q")) {
    resetAll();
    if (isWiFi) {
      sendWiFiResponse("✅ All servos have been reset");
    }
    return;
  }
  
  if (input.equalsIgnoreCase("s")) {
    showStatus();
    return;
  }

  // 检查是否为duration命令
  if (input.startsWith("duration(") && input.endsWith(")")) {
    String durationInput = input.substring(9, input.length() - 1); // 去掉"duration("和")"
    calculateDuration(durationInput);
    return;
  }

  // 批量指令处理 - 先解析所有指令，再统一入队
  int start = 0;
  int commandCount = 0;
  String commands[MAX_QUEUE * NUM_STRUCTS]; // 存储所有解析的指令
  
  // 第一步：解析所有指令
  while (start < input.length()) {
    int end = input.indexOf(';', start);
    if (end == -1) end = input.length();
    String cmd = input.substring(start, end); 
    cmd.trim();
    if (cmd.length() > 0) {
      commands[commandCount++] = cmd;
    }
    start = end + 1;
  }
  
  if (commandCount == 0) return;
  
  // 第二步：统一入队（实现真正的并行）
  // 减少字符串操作，提高性能
  Serial.print("🔄 Batch instruction processing, total ");
  Serial.print(commandCount);
  Serial.println(" instructions");
  if (isWiFi) {
    sendWiFiResponse("🔄 Batch instruction processing, total " + String(commandCount) + " instructions");
  }
  
  // 记录入队成功的指令数量
  int successCount = 0;
  
  for (int i = 0; i < commandCount; i++) {
    if (parseCommand(commands[i], isWiFi)) {
      successCount++;
    }
  }
  
  // 输出批量处理结果
  Serial.print("✅ Batch instruction processing completed, successfully queued ");
  Serial.print(successCount);
  Serial.print("/");
  Serial.println(commandCount);
  if (isWiFi) {
    sendWiFiResponse("✅ Batch instruction processing completed, successfully queued " + String(successCount) + "/" + String(commandCount) + " instructions");
  }
  
  // 关键修改：确保所有机构的动作能够同时开始执行
  if (successCount > 0) {
    unsigned long syncTime = millis();
    
    // 为所有有动作的机构设置同步时间基准
    for (int i = 0; i < NUM_STRUCTS; i++) {
      if (queueSize[i] > 0) {
        // 找到该机构的第一个动作，设置同步时间
        MotionCommand &firstCmd = motionQueue[i][queueStart[i]];
        if (firstCmd.active && firstCmd.currentStep == 0) {
          firstCmd.lastMoveTime = syncTime; // 设置统一的开始时间
          firstCmd.isFirstCommand = true;   // 标记为第一个命令
        }
      }
    }
    
    String syncInfo = "🔄 All structures have set the synchronization time benchmark, ready to execute in parallel";
    Serial.println(syncInfo);
    if (isWiFi) {
      sendWiFiResponse(syncInfo);
    }
  }
  
  // 输出并行执行提示
  String parallelInfo = "🚀 All actions will start executing at the same time, achieving true parallel control";
  Serial.println(parallelInfo);
  if (isWiFi) {
    sendWiFiResponse(parallelInfo);
  }
}
// 处理舵机运动队列
void handleMotionQueues() {
  unsigned long now = millis();
  
  for (int i = 0; i < NUM_STRUCTS; i++) {
    if (queueSize[i] == 0) continue; // 队列为空
    
    MotionCommand &m = motionQueue[i][queueStart[i]];
    if (!m.active) continue; // 跳过无效命令
    
    // 检查执行前延迟
    if (m.preDelaySet && m.currentStep == 0) {
      // 第一个步骤，检查是否需要执行前延迟
      if (now < m.lastMoveTime + m.preDelay) {
        continue; // 等待执行前延迟
      }
      // 延迟结束，开始执行动作
      m.lastMoveTime = now; // 重置时间基准
      m.preDelaySet = false; // 清除延迟标志
    }
    
    // 如果是第一个命令，立即开始执行（无需等待时间）
    if (m.isFirstCommand && m.currentStep == 0) {
      m.lastMoveTime = now;
      m.isFirstCommand = false;
    }
    
    if (now < m.lastMoveTime) continue; // 等待开始时间
    if ((now - m.lastMoveTime) < m.delayPerStep) continue; // 等待步进时间

    m.lastMoveTime = now;
    
    if (m.currentStep >= m.steps) {
      // 当前命令完成，移动到下一个
      m.active = false; // 标记为无效
      queueStart[i] = (queueStart[i] + 1) % MAX_QUEUE;
      queueSize[i]--;
      
      // 如果队列不为空，立即开始下一个命令（无延迟）
      if (queueSize[i] > 0) {
        MotionCommand &nextM = motionQueue[i][queueStart[i]];
        nextM.lastMoveTime = now; // 立即开始，无延迟
        nextM.isFirstCommand = false; // 标记为非第一个命令
      }
      
      // 如果队列为空，输出完成信息
      if (queueSize[i] == 0) {
        String completionMsg = "✅ " + getGroupName(i) + " action sequence completed";
        Serial.println(completionMsg);
        sendWiFiResponse(completionMsg);
      }
      continue;
    }

    // 执行当前步骤 - 使用高级平滑插值计算位置
    float progress = (float)m.currentStep / m.steps;
    
    // 应用高级缓动函数，让运动更自然
    if (m.smoothMotion) {
      // 使用三次贝塞尔曲线缓动函数
      float t = progress;
      float t2 = t * t;
      float t3 = t2 * t;
      progress = 3 * t2 - 2 * t3; // 缓入缓出效果
    }
    
    // 使用线性插值计算精确位置
    int pos1 = m.start1 + (int)((m.target1 - m.start1) * progress + 0.5); // 四舍五入
    int pos2 = m.start2 + (int)((m.target2 - m.start2) * progress + 0.5); // 四舍五入
    
    servos[m.s1_idx].write(pos1);
    servos[m.s2_idx].write(pos2);
    currentPos[m.s1_idx] = pos1;
    currentPos[m.s2_idx] = pos2;
    m.currentStep++;
  }
}

bool enqueue(int group, int target1, int target2, int speed, int preDelay = 0) {
  if (queueSize[group] >= MAX_QUEUE) {
    String errorMsg = "❌ " + getGroupName(group) + " queue is full";
    Serial.println(errorMsg);
    sendWiFiResponse(errorMsg);
    return false;
  }
  
  int s1 = group * 2;
  int s2 = group * 2 + 1;
  
  // 确定起始位置：如果队列不为空，使用上一个命令的目标位置；否则使用当前位置
  int start1, start2;
  if (queueSize[group] > 0) {
    // 队列不为空，使用上一个命令的目标位置作为起始位置
    int lastIndex = (queueEnd[group] - 1 + MAX_QUEUE) % MAX_QUEUE;
    start1 = motionQueue[group][lastIndex].target1;
    start2 = motionQueue[group][lastIndex].target2;
  } else {
    // 队列为空，使用当前位置
    start1 = currentPos[s1];
    start2 = currentPos[s2];
  }
  
  // 增加步数，让运动更平滑
  int steps = max(abs(target1 - start1), abs(target2 - start2));
  steps = max(steps, 5); // 至少5步，让运动更平滑

  // 关键修改：为批量指令设置统一的时间基准
  unsigned long baseTime = millis();
  if (queueSize[group] == 0) {
    // 队列为空，这是该机构的第一个动作，设置基准时间
    baseTime = millis();
  } else {
    // 队列不为空，使用当前时间作为基准（确保与队列中其他动作同步）
    baseTime = millis();
  }

  MotionCommand m = {
    s1, s2,
    start1, target1,
    start2, target2,
    steps, 0,
    speedLevels[speed], baseTime, // 使用基准时间初始化
    true,  // 标记为激活
    true,  // 启用平滑运动
    preDelay,  // 执行前延迟
    preDelay > 0,  // 是否设置了执行前延迟
    (queueSize[group] == 0)  // 如果队列为空，标记为第一个命令
  };

  motionQueue[group][queueEnd[group]] = m;
  queueEnd[group] = (queueEnd[group] + 1) % MAX_QUEUE;
  queueSize[group]++;
  
  return true;
}

bool parseCommand(String input, bool isWiFi = false) {
  if (input.length() < 2) {
    Serial.println("❌ Command format error");
    if (isWiFi) {
      sendWiFiResponse("❌ Command format error");
    }
    return false;
  }
  
  int group = -1;
  String groupStr = "";
  
  // 检查是否为2字符的结构标识
  if (input.length() >= 2) {
    groupStr = input.substring(0, 2);
    if (groupStr.equalsIgnoreCase("RF")) group = 0;      // 右前脚
    else if (groupStr.equalsIgnoreCase("LF")) group = 1; // 左前脚
    else if (groupStr.equalsIgnoreCase("RR")) group = 2; // 右后脚
    else if (groupStr.equalsIgnoreCase("LR")) group = 3; // 左后脚
    else if (groupStr.equalsIgnoreCase("TA")) group = 4; // 尾巴
    else if (groupStr.equalsIgnoreCase("RE")) group = 5; // 右耳
    else if (groupStr.equalsIgnoreCase("LE")) group = 6; // 左耳
    else if (groupStr.equalsIgnoreCase("HD")) group = 7; // 头部
  }
  
  // 如果2字符不匹配，尝试1字符（兼容旧格式）
  if (group == -1 && input.length() >= 1) {
    group = input.charAt(0) - '0';
    if (group >= 0 && group < NUM_STRUCTS) {
      groupStr = String(group);
    }
  }
  
  if (group < 0 || group >= NUM_STRUCTS) {
    Serial.println("❌ Structure identifier error, should be: RF/LF/RR/LR/TA/RE/LE/HD or 0-7");
    if (isWiFi) {
      sendWiFiResponse("❌ Structure identifier error, should be: RF/LF/RR/LR/TA/RE/LE/HD or 0-7");
    }
    return false;
  }

  String cmd = input.substring(groupStr.length());
  cmd.trim();
  
  // 检查是否为新的指令格式：<机构名> F<动作角度> R<弯曲角度>
  if (cmd.indexOf('F') != -1 && cmd.indexOf('R') != -1) {
    return parseNewCommand(group, cmd, isWiFi);
  }
  
  // 兼容旧格式
  bool hasW = false;
  if (cmd.endsWith("w") || cmd.endsWith("W")) {
    hasW = true;
    cmd = cmd.substring(0, cmd.length() - 1);
  }

  if (!cmd.startsWith("u")) {
    Serial.println("❌ Command format error, should start with u or use new format F<angle> R<angle>");
    if (isWiFi) {
      sendWiFiResponse("❌ Command format error, should start with u or use new format F<angle> R<angle>");
    }
    return false;
  }
  
  cmd = cmd.substring(1);
  int c1 = cmd.indexOf(',');
  int c2 = cmd.lastIndexOf(',');
  if (c1 == -1 || c2 == -1 || c1 == c2) {
    Serial.println("❌ Parameter format error, should be u servo1 rotation angle, servo2 rotation angle, speed");
    if (isWiFi) {
      sendWiFiResponse("❌ Parameter format error, should be u servo1 rotation angle, servo2 rotation angle, speed");
    }
    return false;
  }

  int rotation1 = cmd.substring(0, c1).toInt();
  int rotation2 = cmd.substring(c1 + 1, c2).toInt();
  int speed = constrain(cmd.substring(c2 + 1).toInt(), 1, 5);

  // 旋转角范围：-90到+90，转换为舵机角度：0到180
  int angle1 = constrain(mid + rotation1, 0, 180);
  int angle2 = constrain(mid + rotation2, 0, 180);
  
  bool success = enqueue(group, angle1, angle2, speed);
  if (success) {
    String response = "▶ " + getGroupName(group) + " add action → servo1:" + String(angle1) + "(rotation:" + String(rotation1) + "°), servo2:" + String(angle2) + "(rotation:" + String(rotation2) + "°), speed:" + String(speed);
    Serial.println(response);
    if (isWiFi) {
      sendWiFiResponse(response);
    }
  }

  if (hasW && success) {
    // 对称动作：两个舵机角度互换
    int angle1_sym = angle2;
    int angle2_sym = angle1;
    
    if (enqueue(group, angle1_sym, angle2_sym, speed)) {
      String response = "↔ " + getGroupName(group) + " add symmetric action → servo1:" + String(angle1_sym) + "(rotation:" + String(rotation2) + "°), servo2:" + String(angle2_sym) + "(rotation:" + String(rotation1) + "°), speed:" + String(speed);
      Serial.println(response);
      if (isWiFi) {
        sendWiFiResponse(response);
      }
    }
  }
  
  return success;
}

void resetAll() {
  Serial.println("↩️ All servos are being reset in parallel...");
  sendWiFiResponse("↩️ All servos are being reset in parallel...");
  
  // 清空所有队列
  for (int i = 0; i < NUM_STRUCTS; i++) {
    queueStart[i] = 0;
    queueEnd[i] = 0;
    queueSize[i] = 0;
    for (int j = 0; j < MAX_QUEUE; j++) {
      motionQueue[i][j].active = false;
      motionQueue[i][j].isFirstCommand = false;
    }
  }
  
  // 添加复位动作到所有队列（并行执行）
  for (int i = 0; i < NUM_STRUCTS; i++) {
    enqueue(i, mid, mid, 2);  // 用慢速复位，所有机构同时开始
  }
  
  Serial.println("✅ The parallel reset action has been sent to all queues");
  Serial.println("🔄 All structures will start resetting at the same time, without waiting");
  sendWiFiResponse("✅ The parallel reset action has been sent to all queues");
  sendWiFiResponse("🔄 All structures will start resetting at the same time, without waiting");
}

String getGroupName(int group) {
  switch(group) {
    case 0: return "Right Front Leg(RF)";
    case 1: return "Left Front Leg(LF)";
    case 2: return "Right Rear Leg(RR)";
    case 3: return "Left Rear Leg(LR)";
    case 4: return "Tail(TA)";
    case 5: return "Right Ear(RE)";
    case 6: return "Left Ear(LE)";
    case 7: return "Head(HD)";
    default: return "Unknown structure";
  }
}

// 解析新指令格式：F<动作角度> R<弯曲角度>
bool parseNewCommand(int group, String cmd, bool isWiFi = false) {
  int forwardAngle = 0;
  int bendAngle = 0;
  int speed = 3; // 默认速度
  int preDelay = 0; // 执行前延迟
  
  // 解析F参数（动作角度）
  int fIndex = cmd.indexOf('F');
  if (fIndex != -1) {
    int fEnd = cmd.indexOf(' ', fIndex);
    if (fEnd == -1) fEnd = cmd.indexOf('R', fIndex);
    if (fEnd == -1) fEnd = cmd.length();
    
    String fStr = cmd.substring(fIndex + 1, fEnd);
    forwardAngle = fStr.toInt();
  }
  
  // 解析R参数（弯曲角度）
  int rIndex = cmd.indexOf('R');
  if (rIndex != -1) {
    int rEnd = cmd.indexOf(' ', rIndex);
    if (rEnd == -1) rEnd = cmd.length();
    
    String rStr = cmd.substring(rIndex + 1, rEnd);
    bendAngle = rStr.toInt();
  }
  
  // 解析速度（如果存在）
  int speedIndex = cmd.indexOf('S');
  if (speedIndex != -1) {
    int speedEnd = cmd.indexOf(' ', speedIndex);
    if (speedEnd == -1) speedEnd = cmd.indexOf('D', speedIndex);
    if (speedEnd == -1) speedEnd = cmd.length();
    
    String speedStr = cmd.substring(speedIndex + 1, speedEnd);
    speed = constrain(speedStr.toInt(), 1, 5);
  }
  
  // 解析延迟参数（如果存在）
  int delayIndex = cmd.indexOf('D');
  if (delayIndex != -1) {
    int delayEnd = cmd.indexOf(' ', delayIndex);
    if (delayEnd == -1) delayEnd = cmd.length();
    
    String delayStr = cmd.substring(delayIndex + 1, delayEnd);
    preDelay = delayStr.toInt();
  }
  
  // 根据机构类型和角度计算舵机角度
  int rotation1, rotation2;
  calculateServoAngles(group, forwardAngle, bendAngle, rotation1, rotation2);
  
  // 转换为舵机角度，对耳朵和头部限制在45度以内
  int angle1, angle2;
  if (group >= 5) { // 耳朵和头部结构
    angle1 = constrain(mid + rotation1, mid - 45, mid + 45);
    angle2 = constrain(mid + rotation2, mid - 45, mid + 45);
  } else { // 脚部结构
    angle1 = constrain(mid + rotation1, 0, 180);
    angle2 = constrain(mid + rotation2, 0, 180);
  }
  
  bool success = enqueue(group, angle1, angle2, speed, preDelay);
  if (success) {
    String response = "▶ " + getGroupName(group);
    
    // 根据结构类型输出不同的参数名称
    switch(group) {
      case 0: // RF
      case 1: // LF
      case 2: // RR
      case 3: // LR
        response += " add action → Lift/Drop:" + String(forwardAngle) + "°, Flexion:" + String(bendAngle) + "°";
        break;
      case 4: // TA
        response += " add action → Wag:" + String(forwardAngle) + "°, Flexion:" + String(bendAngle) + "°";
        break;
      case 5: // RE
      case 6: // LE
        response += " add action → Rotation:" + String(forwardAngle) + "°, Pitch:" + String(bendAngle) + "°";
        break;
      case 7: // HD
        response += " add action → Rotation:" + String(forwardAngle) + "°, Pitch:" + String(bendAngle) + "°";
        break;
      default:
        response += " add action → Forward:" + String(forwardAngle) + "°, Flexion:" + String(bendAngle) + "°";
        break;
    }
    
    response += " → servo1:" + String(angle1) + "(rotation:" + String(rotation1) + "°), servo2:" + String(angle2) + "(rotation:" + String(rotation2) + "°), speed:" + String(speed);
    if (preDelay > 0) {
      response += ", delay:" + String(preDelay) + "ms";
    }
    
    Serial.println(response);
    if (isWiFi) {
      sendWiFiResponse(response);
    }
  }
  
  return success;
}

// 根据机构类型和动作角度计算舵机旋转角度
void calculateServoAngles(int group, int forwardAngle, int bendAngle, int &rotation1, int &rotation2) {
  switch(group) {
    case 0: // 右前脚(RF) - F为正值=抬起(后退)，F为负值=放下(前进)
      rotation1 = -forwardAngle + bendAngle;  // 抬起时后退，放下时前进
      rotation2 = -forwardAngle - bendAngle;
      break;
      
    case 1: // 左前脚(LF) - F为正值=抬起(后退)，F为负值=放下(前进)
      rotation1 = forwardAngle + bendAngle;   // 抬起时后退，放下时前进
      rotation2 = forwardAngle - bendAngle;
      break;
      
    case 2: // 右后脚(RR) - F为正值=抬起(前进)，F为负值=放下(后退)
      rotation1 = forwardAngle - bendAngle;   // 抬起时前进，放下时后退，R参数修正
      rotation2 = forwardAngle + bendAngle;
      break;
      
    case 3: // 左后脚(LR) - F为正值=抬起(前进)，F为负值=放下(后退)
      rotation1 = -forwardAngle + bendAngle;  // 抬起时前进，放下时后退
      rotation2 = -forwardAngle - bendAngle;
      break;
      
    case 4: // 尾巴(TA) - F为正值=顺时针摇摆，F为负值=逆时针摇摆
      rotation1 = forwardAngle + bendAngle;   // 顺时针摇摆
      rotation2 = forwardAngle - bendAngle;
      break;
      
    case 5: // 右耳(RE)
      // 右耳：舵机1控制旋转，舵机2控制俯仰
      rotation1 = forwardAngle;  // 旋转角度
      rotation2 = bendAngle;     // 俯仰角度
      break;
      
    case 6: // 左耳(LE)
      // 左耳：舵机1控制旋转，舵机2控制俯仰
      rotation1 = forwardAngle;  // 旋转角度
      rotation2 = -bendAngle;     // 俯仰角度
      break;
      
    case 7: // 头部(HD)
      // 头部：舵机1控制左右转动，舵机2控制上下点头
      rotation1 = forwardAngle;  // 左右转动角度
      rotation2 = -bendAngle;     // 上下点头角度
      break;
      
    default:
      rotation1 = 0;
      rotation2 = 0;
      break;
  }
}

void showStatus() {
  Serial.println("\n📊 System status:");
  sendWiFiResponse("\n📊 System status:");
  
  // 显示并行执行模式状态
  Serial.println("🔄 Parallel execution mode: activated");
  Serial.println("⚡ Command delay: 0ms (no delay parallel execution)");
  sendWiFiResponse("🔄 Parallel execution mode: activated");
  sendWiFiResponse("⚡ Command delay: 0ms (no delay parallel execution)");
  
  for (int i = 0; i < NUM_STRUCTS; i++) {
    String status = getGroupName(i) + ": Queue size=" + String(queueSize[i]) + ", servo1=" + String(currentPos[i*2]) + ", servo2=" + String(currentPos[i*2+1]);
    Serial.println(status);
    sendWiFiResponse(status);
  }
  Serial.println();
  sendWiFiResponse("");
}

// 计算指令执行时间的函数
void calculateDuration(String input) {
  Serial.println("⏱️ Calculate command execution time...");
  sendWiFiResponse("⏱️ Calculate command execution time...");
  
  // 临时存储每个结构的动作时间
  unsigned long groupDurations[NUM_STRUCTS] = {0};
  unsigned long groupStartTimes[NUM_STRUCTS] = {0};
  bool groupHasActions[NUM_STRUCTS] = {false};
  
  // 解析指令
  int start = 0;
  int commandIndex = 0;
  
  while (start < input.length()) {
    int end = input.indexOf(';', start);
    if (end == -1) end = input.length();
    String cmd = input.substring(start, end); 
    cmd.trim();
    
    if (cmd.length() > 0) {
      commandIndex++;
      String cmdInfo = "  " + String(commandIndex) + ". " + cmd;
      Serial.println(cmdInfo);
      sendWiFiResponse(cmdInfo);
      
      // 解析单个指令
      unsigned long duration = parseCommandDuration(cmd);
      int group = getCommandGroup(cmd);
      
      if (group >= 0 && group < NUM_STRUCTS) {
        if (!groupHasActions[group]) {
          groupStartTimes[group] = 0; // 第一个动作从0开始
          groupHasActions[group] = true;
        }
        
        // 累加该结构的执行时间
        groupDurations[group] += duration;
        
        String durationInfo = "     → " + getGroupName(group) + " Estimated time: " + String(duration) + "ms";
        Serial.println(durationInfo);
        sendWiFiResponse(durationInfo);
      }
    }
    start = end + 1;
  }
  
  // 计算总执行时间（考虑并发执行）
  unsigned long totalDuration = 0;
  Serial.println("\n📊 Execution time analysis:");
  sendWiFiResponse("\n📊 Execution time analysis:");
  
  for (int i = 0; i < NUM_STRUCTS; i++) {
    if (groupHasActions[i]) {
      String analysis = "  " + getGroupName(i) + ": " + String(groupDurations[i]) + "ms";
      Serial.println(analysis);
      sendWiFiResponse(analysis);
      totalDuration = max(totalDuration, groupDurations[i]);
    }
  }
  
  String totalInfo = "\n⏱️ Total execution time: " + String(totalDuration) + "ms";
  String totalSecInfo = "⏱️ Total execution time: " + String(totalDuration / 1000.0, 1) + " seconds";
  Serial.println(totalInfo);
  Serial.println(totalSecInfo);
  Serial.println();
  sendWiFiResponse(totalInfo);
  sendWiFiResponse(totalSecInfo);
  sendWiFiResponse("");
}

// 解析单个指令的执行时间
unsigned long parseCommandDuration(String cmd) {
  int group = getCommandGroup(cmd);
  if (group < 0) return 0;
  
  // 解析新格式：F<动作角度> R<弯曲角度> S<速度> D<延迟>
  if (cmd.indexOf('F') != -1 && cmd.indexOf('R') != -1) {
    return parseNewCommandDuration(group, cmd);
  }
  
  // 解析旧格式：u<舵机1旋转角>,<舵机2旋转角>,<速度>
  if (cmd.indexOf('u') != -1) {
    return parseOldCommandDuration(group, cmd);
  }
  
  return 0;
}

// 获取指令对应的结构组号
int getCommandGroup(String cmd) {
  // 检查是否为2字符的结构标识
  if (cmd.length() >= 2) {
    String groupStr = cmd.substring(0, 2);
    if (groupStr.equalsIgnoreCase("RF")) return 0;      // 右前脚
    else if (groupStr.equalsIgnoreCase("LF")) return 1; // 左前脚
    else if (groupStr.equalsIgnoreCase("RR")) return 2; // 右后脚
    else if (groupStr.equalsIgnoreCase("LR")) return 3; // 左后脚
    else if (groupStr.equalsIgnoreCase("TA")) return 4; // 尾巴
    else if (groupStr.equalsIgnoreCase("RE")) return 5; // 右耳
    else if (groupStr.equalsIgnoreCase("LE")) return 6; // 左耳
    else if (groupStr.equalsIgnoreCase("HD")) return 7; // 头部
  }
  
  // 检查是否为1字符的结构标识
  if (cmd.length() >= 1) {
    int group = cmd.charAt(0) - '0';
    if (group >= 0 && group < NUM_STRUCTS) {
      return group;
    }
  }
  
  return -1;
}

// 解析新格式指令的执行时间
unsigned long parseNewCommandDuration(int group, String cmd) {
  int forwardAngle = 0;
  int bendAngle = 0;
  int speed = 3; // 默认速度
  int preDelay = 0; // 执行前延迟
  
  // 解析F参数（动作角度）
  int fIndex = cmd.indexOf('F');
  if (fIndex != -1) {
    int fEnd = cmd.indexOf(' ', fIndex);
    if (fEnd == -1) fEnd = cmd.indexOf('R', fIndex);
    if (fEnd == -1) fEnd = cmd.indexOf('S', fIndex);
    if (fEnd == -1) fEnd = cmd.indexOf('D', fIndex);
    if (fEnd == -1) fEnd = cmd.length();
    
    String fStr = cmd.substring(fIndex + 1, fEnd);
    forwardAngle = fStr.toInt();
  }
  
  // 解析R参数（弯曲角度）
  int rIndex = cmd.indexOf('R');
  if (rIndex != -1) {
    int rEnd = cmd.indexOf(' ', rIndex);
    if (rEnd == -1) rEnd = cmd.indexOf('S', rIndex);
    if (rEnd == -1) rEnd = cmd.indexOf('D', rIndex);
    if (rEnd == -1) rEnd = cmd.length();
    
    String rStr = cmd.substring(rIndex + 1, rEnd);
    bendAngle = rStr.toInt();
  }
  
  // 解析速度（如果存在）
  int speedIndex = cmd.indexOf('S');
  if (speedIndex != -1) {
    int speedEnd = cmd.indexOf(' ', speedIndex);
    if (speedEnd == -1) speedEnd = cmd.indexOf('D', speedIndex);
    if (speedEnd == -1) speedEnd = cmd.length();
    
    String speedStr = cmd.substring(speedIndex + 1, speedEnd);
    speed = constrain(speedStr.toInt(), 1, 5);
  }
  
  // 解析延迟参数（如果存在）
  int delayIndex = cmd.indexOf('D');
  if (delayIndex != -1) {
    int delayEnd = cmd.indexOf(' ', delayIndex);
    if (delayEnd == -1) delayEnd = cmd.length();
    
    String delayStr = cmd.substring(delayIndex + 1, delayEnd);
    preDelay = delayStr.toInt();
  }
  
  // 计算舵机角度
  int rotation1, rotation2;
  calculateServoAngles(group, forwardAngle, bendAngle, rotation1, rotation2);
  
  // 转换为舵机角度
  int angle1, angle2;
  if (group >= 5) { // 耳朵和头部结构
    angle1 = constrain(mid + rotation1, mid - 45, mid + 45);
    angle2 = constrain(mid + rotation2, mid - 45, mid + 45);
  } else { // 脚部结构
    angle1 = constrain(mid + rotation1, 0, 180);
    angle2 = constrain(mid + rotation2, 0, 180);
  }
  
  // 计算步数和执行时间（使用当前位置作为起始位置）
  int start1 = currentPos[group * 2];
  int start2 = currentPos[group * 2 + 1];
  int steps = max(abs(angle1 - start1), abs(angle2 - start2));
  steps = max(steps, 5); // 至少5步
  
  unsigned long duration = steps * speedLevels[speed] + preDelay;
  return duration;
}

// 解析旧格式指令的执行时间
unsigned long parseOldCommandDuration(int group, String cmd) {
  if (!cmd.startsWith("u")) return 0;
  
  cmd = cmd.substring(1); // 去掉'u'
  int c1 = cmd.indexOf(',');
  int c2 = cmd.lastIndexOf(',');
  
  if (c1 == -1 || c2 == -1 || c1 == c2) return 0;
  
  int rotation1 = cmd.substring(0, c1).toInt();
  int rotation2 = cmd.substring(c1 + 1, c2).toInt();
  int speed = constrain(cmd.substring(c2 + 1).toInt(), 1, 5);
  
  // 转换为舵机角度
  int angle1 = constrain(mid + rotation1, 0, 180);
  int angle2 = constrain(mid + rotation2, 0, 180);
  
  // 计算步数和执行时间（使用当前位置作为起始位置）
  int start1 = currentPos[group * 2];
  int start2 = currentPos[group * 2 + 1];
  int steps = max(abs(angle1 - start1), abs(angle2 - start2));
  steps = max(steps, 5); // 至少5步
  
  unsigned long duration = steps * speedLevels[speed];
  return duration;
}
