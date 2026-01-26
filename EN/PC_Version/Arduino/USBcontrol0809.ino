// 多结构并发控制 + 多动作排队执行版本
#include <Servo.h>

const int NUM_STRUCTS = 8;  // 增加到8个结构（5个脚+3个头部结构）
const int MAX_QUEUE = 10;  // 增加队列容量
const int mid = 90;
int speedLevels[6] = {0, 30, 20, 10, 5, 2};
const int COMMAND_DELAY = 100; // 命令间延迟100ms

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

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < NUM_STRUCTS * 2; i++) {
    servos[i].attach(pins[i]);
    currentPos[i] = mid;
  }
  
  // 初始化队列
  for (int i = 0; i < NUM_STRUCTS; i++) {
    for (int j = 0; j < MAX_QUEUE; j++) {
      motionQueue[i][j].active = false;
    }
  }
  
  Serial.println("✅ 系统启动，支持多结构 + 多动作排队执行 + 平滑运动优化");
  Serial.println("🎯 支持指令格式：");
  Serial.println("  ▶ 新格式：RF F30 R20 S3");
  Serial.println("  ▶ 旧格式：RFu40,-20,3");
  Serial.println("  ▶ 多动作：RF F30 R20;LF F-30 R15;RR F25 R-10");
  Serial.println("  ▶ 复位：q");
  Serial.println("  ▶ 查看状态：s");
  Serial.println("  ▶ 结构对应：RF(右前脚) LF(左前脚) RR(右后脚) LR(左后脚) TA(尾巴) RE(右耳) LE(左耳) HD(头部)");
  Serial.println("  ▶ 新格式说明：");
  Serial.println("    - RF/LF: F<抬起/放下> R<弯曲角度> (F正值=抬起, F负值=放下)");
  Serial.println("    - RR/LR: F<抬起/放下> R<弯曲角度> (F正值=抬起, F负值=放下)");
  Serial.println("    - TA: F<摇摆> R<弯曲角度> (F正值=顺时针摇摆, F负值=逆时针摇摆)");
  Serial.println("    - RE/LE: F<旋转角度> R<俯仰角度>");
  Serial.println("    - HD: F<左右转动> R<上下点头>");
  Serial.println("  ▶ 延迟功能：D<延迟ms> 在动作执行前等待指定时间");
  Serial.println("  ▶ 时间计算：duration(RF F40 R20 S5;RF F-40 R20 S5 D1000;RE F-40 R20 S5) 计算指令执行时间");
}

void loop() {
  handleSerial();
  handleMotionQueues();
}
// 处理串口输入
void handleSerial() {
  if (!Serial.available()) return;
  String input = Serial.readStringUntil('\n');
  input.trim(); 
  input.replace("；", ";");

  if (input.equalsIgnoreCase("q")) {
    resetAll();
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

  int start = 0;
  while (start < input.length()) {
    int end = input.indexOf(';', start);
    if (end == -1) end = input.length();
    String cmd = input.substring(start, end); 
    cmd.trim();
    if (cmd.length() > 0) parseCommand(cmd);
    start = end + 1;
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
    
    if (now < m.lastMoveTime) continue; // 等待开始时间
    if ((now - m.lastMoveTime) < m.delayPerStep) continue; // 等待步进时间

    m.lastMoveTime = now;
    
    if (m.currentStep >= m.steps) {
      // 当前命令完成，移动到下一个
      m.active = false; // 标记为无效
      queueStart[i] = (queueStart[i] + 1) % MAX_QUEUE;
      queueSize[i]--;
      
      // 如果队列不为空，初始化下一个命令的时间（添加延迟）
      if (queueSize[i] > 0) {
        MotionCommand &nextM = motionQueue[i][queueStart[i]];
        nextM.lastMoveTime = now + COMMAND_DELAY; // 设置下一个命令的开始时间，添加延迟
      }
      
      // 如果队列为空，输出完成信息
      if (queueSize[i] == 0) {
        Serial.print("✅ "); Serial.print(getGroupName(i)); Serial.println(" 动作序列完成");
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
    Serial.print("❌ "); Serial.print(getGroupName(group)); Serial.println(" 队列已满");
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

  MotionCommand m = {
    s1, s2,
    start1, target1,
    start2, target2,
    steps, 0,
    speedLevels[speed], millis(), // 使用当前时间初始化
    true,  // 标记为激活
    true,  // 启用平滑运动
    preDelay,  // 执行前延迟
    preDelay > 0  // 是否设置了执行前延迟
  };

  motionQueue[group][queueEnd[group]] = m;
  queueEnd[group] = (queueEnd[group] + 1) % MAX_QUEUE;
  queueSize[group]++;
  
  return true;
}

void parseCommand(String input) {
  if (input.length() < 2) {
    Serial.println("❌ 指令格式错误");
    return;
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
    Serial.println("❌ 结构标识错误，应为：RF/LF/RR/LR/TA/RE/LE/HD 或 0-7");
    return;
  }

  String cmd = input.substring(groupStr.length());
  cmd.trim();
  
  // 检查是否为新的指令格式：<机构名> F<动作角度> R<弯曲角度>
  if (cmd.indexOf('F') != -1 && cmd.indexOf('R') != -1) {
    parseNewCommand(group, cmd);
    return;
  }
  
  // 兼容旧格式
  bool hasW = false;
  if (cmd.endsWith("w") || cmd.endsWith("W")) {
    hasW = true;
    cmd = cmd.substring(0, cmd.length() - 1);
  }

  if (!cmd.startsWith("u")) {
    Serial.println("❌ 指令格式错误，应以u开头或使用新格式 F<角度> R<角度>");
    return;
  }
  
  cmd = cmd.substring(1);
  int c1 = cmd.indexOf(',');
  int c2 = cmd.lastIndexOf(',');
  if (c1 == -1 || c2 == -1 || c1 == c2) {
    Serial.println("❌ 参数格式错误，应为 u舵机1旋转角,舵机2旋转角,速度");
    return;
  }

  int rotation1 = cmd.substring(0, c1).toInt();
  int rotation2 = cmd.substring(c1 + 1, c2).toInt();
  int speed = constrain(cmd.substring(c2 + 1).toInt(), 1, 5);

  // 旋转角范围：-90到+90，转换为舵机角度：0到180
  int angle1 = constrain(mid + rotation1, 0, 180);
  int angle2 = constrain(mid + rotation2, 0, 180);
  
  if (enqueue(group, angle1, angle2, speed)) {
    Serial.print("▶ "); Serial.print(getGroupName(group)); 
    Serial.print(" 添加动作 → 舵机1:"); Serial.print(angle1);
    Serial.print("(旋转:"); Serial.print(rotation1); Serial.print("°)");
    Serial.print(", 舵机2:"); Serial.print(angle2);
    Serial.print("(旋转:"); Serial.print(rotation2); Serial.print("°)");
    Serial.print(", 速度:"); Serial.println(speed);
  }

  if (hasW) {
    // 对称动作：两个舵机角度互换
    int angle1_sym = angle2;
    int angle2_sym = angle1;
    
    if (enqueue(group, angle1_sym, angle2_sym, speed)) {
      Serial.print("↔ "); Serial.print(getGroupName(group)); 
      Serial.print(" 添加对称动作 → 舵机1:"); Serial.print(angle1_sym);
      Serial.print("(旋转:"); Serial.print(rotation2); Serial.print("°)");
      Serial.print(", 舵机2:"); Serial.print(angle2_sym);
      Serial.print("(旋转:"); Serial.print(rotation1); Serial.print("°)");
      Serial.print(", 速度:"); Serial.println(speed);
    }
  }
}

void resetAll() {
  Serial.println("↩️ 所有舵机复位中...");
  
  // 清空所有队列
  for (int i = 0; i < NUM_STRUCTS; i++) {
    queueStart[i] = 0;
    queueEnd[i] = 0;
    queueSize[i] = 0;
    for (int j = 0; j < MAX_QUEUE; j++) {
      motionQueue[i][j].active = false;
    }
  }
  
  // 添加复位动作到所有队列
  for (int i = 0; i < NUM_STRUCTS; i++) {
    enqueue(i, mid, mid, 2);  // 用慢速复位
  }
  
  Serial.println("✅ 已发送复位动作至所有队列\n");
}

String getGroupName(int group) {
  switch(group) {
    case 0: return "右前脚(RF)";
    case 1: return "左前脚(LF)";
    case 2: return "右后脚(RR)";
    case 3: return "左后脚(LR)";
    case 4: return "尾巴(TA)";
    case 5: return "右耳(RE)";
    case 6: return "左耳(LE)";
    case 7: return "头部(HD)";
    default: return "未知结构";
  }
}

// 解析新指令格式：F<动作角度> R<弯曲角度>
void parseNewCommand(int group, String cmd) {
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
  
  if (enqueue(group, angle1, angle2, speed, preDelay)) {
    Serial.print("▶ "); Serial.print(getGroupName(group)); 
    
    // 根据结构类型输出不同的参数名称
    switch(group) {
      case 0: // RF
      case 1: // LF
      case 2: // RR
      case 3: // LR
        Serial.print(" 添加动作 → 抬起/放下:"); Serial.print(forwardAngle);
        Serial.print("°, 弯曲:"); Serial.print(bendAngle);
        break;
      case 4: // TA
        Serial.print(" 添加动作 → 摇摆:"); Serial.print(forwardAngle);
        Serial.print("°, 弯曲:"); Serial.print(bendAngle);
        break;
      case 5: // RE
      case 6: // LE
        Serial.print(" 添加动作 → 旋转:"); Serial.print(forwardAngle);
        Serial.print("°, 俯仰:"); Serial.print(bendAngle);
        break;
      case 7: // HD
        Serial.print(" 添加动作 → 左右转动:"); Serial.print(forwardAngle);
        Serial.print("°, 上下点头:"); Serial.print(bendAngle);
        break;
      default:
        Serial.print(" 添加动作 → 前方:"); Serial.print(forwardAngle);
        Serial.print("°, 弯曲:"); Serial.print(bendAngle);
        break;
    }
    
    Serial.print("° → 舵机1:"); Serial.print(angle1);
    Serial.print("(旋转:"); Serial.print(rotation1); Serial.print("°)");
    Serial.print(", 舵机2:"); Serial.print(angle2);
    Serial.print("(旋转:"); Serial.print(rotation2); Serial.print("°)");
    Serial.print(", 速度:"); Serial.print(speed);
    if (preDelay > 0) {
      Serial.print(", 延迟:"); Serial.print(preDelay); Serial.print("ms");
    }
    Serial.println();
  }
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
      rotation1 = forwardAngle + bendAngle;   // 抬起时前进，放下时后退
      rotation2 = forwardAngle - bendAngle;
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
      rotation2 = bendAngle;     // 俯仰角度
      break;
      
    case 7: // 头部(HD)
      // 头部：舵机1控制左右转动，舵机2控制上下点头
      rotation1 = forwardAngle;  // 左右转动角度
      rotation2 = bendAngle;     // 上下点头角度
      break;
      
    default:
      rotation1 = 0;
      rotation2 = 0;
      break;
  }
}

void showStatus() {
  Serial.println("\n📊 系统状态：");
  for (int i = 0; i < NUM_STRUCTS; i++) {
    Serial.print(getGroupName(i)); 
    Serial.print(": 队列中动作数="); Serial.print(queueSize[i]);
    Serial.print(", 舵机1="); Serial.print(currentPos[i*2]);
    Serial.print(", 舵机2="); Serial.print(currentPos[i*2+1]);
    Serial.println();
  }
  Serial.println();
}

// 计算指令执行时间的函数
void calculateDuration(String input) {
  Serial.println("⏱️ 计算指令执行时间...");
  
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
      Serial.print("  "); Serial.print(commandIndex); Serial.print(". "); Serial.println(cmd);
      
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
        
        Serial.print("     → "); Serial.print(getGroupName(group)); 
        Serial.print(" 预估时间: "); Serial.print(duration); Serial.println("ms");
      }
    }
    start = end + 1;
  }
  
  // 计算总执行时间（考虑并发执行）
  unsigned long totalDuration = 0;
  Serial.println("\n📊 执行时间分析：");
  
  for (int i = 0; i < NUM_STRUCTS; i++) {
    if (groupHasActions[i]) {
      Serial.print("  "); Serial.print(getGroupName(i)); 
      Serial.print(": "); Serial.print(groupDurations[i]); Serial.println("ms");
      totalDuration = max(totalDuration, groupDurations[i]);
    }
  }
  
  Serial.print("\n⏱️ 总执行时间: "); Serial.print(totalDuration); Serial.println("ms");
  Serial.print("⏱️ 总执行时间: "); Serial.print(totalDuration / 1000.0, 1); Serial.println("秒");
  Serial.println();
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