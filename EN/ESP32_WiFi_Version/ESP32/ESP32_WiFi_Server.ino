/*
 * ESP32 WiFi指令转发器 - 用于Arduino Mega2560舵机控制系统
 * 
 * 硬件连接：
 * ESP32 GPIO16 -> Arduino Mega2560 RX1 (19)
 * ESP32 GPIO17 -> Arduino Mega2560 TX1 (18)
 * ESP32 VIN -> Arduino Mega2560 5V
 * ESP32 GND -> Arduino Mega2560 GND
 * 
 * 功能：作为WiFi热点，接收PC发送的指令并转发给Arduino
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

// WiFi配置
const char* ssid = "RobotControl";        // WiFi热点名称
const char* password = "12345678";        // WiFi密码

// IP地址配置（可选：如果需要固定IP地址，取消下面的注释）
// IPAddress local_IP(192, 168, 4, 1);     // ESP32的IP地址
// IPAddress gateway(192, 168, 4, 1);        // 网关地址（通常与IP相同）
// IPAddress subnet(255, 255, 255, 0);      // 子网掩码

// 服务器配置
const int serverPort = 80;
WebServer server(serverPort);

// 串口配置
#define SERIAL_BAUD_RATE 115200
#define ARDUINO_RX_PIN 16  // ESP32的GPIO16连接到Arduino的TX1
#define ARDUINO_TX_PIN 17  // ESP32的GPIO17连接到Arduino的RX1

// 创建硬件串口对象
HardwareSerial ArduinoSerial(2); // 使用UART2

// 状态变量
String lastCommand = "";
String lastResponse = "";

void setup() {
  // 初始化调试串口
  Serial.begin(SERIAL_BAUD_RATE);
  
  // 初始化与Arduino通信的串口
  ArduinoSerial.begin(SERIAL_BAUD_RATE, SERIAL_8N1, ARDUINO_RX_PIN, ARDUINO_TX_PIN);
  
  Serial.println("ESP32 WiFi指令转发器启动中...");
  
  // 创建WiFi热点
  // 如果需要固定IP地址，取消下面的注释并使用固定IP配置
  // if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
  //   Serial.println("IP配置失败，使用默认IP");
  // }
  WiFi.softAP(ssid, password);
  Serial.print("WiFi热点已创建: ");
  Serial.println(ssid);
  Serial.print("密码: ");
  Serial.println(password);
  Serial.print("IP地址: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("注意：ESP32的IP地址通常是 192.168.4.1（默认值）");
  Serial.println("如果IP地址不同，请使用串口监视器显示的IP地址");
  
  // 设置Web服务器路由
  setupWebServer();
  
  // 启动Web服务器
  server.begin();
  Serial.println("Web服务器已启动");
  Serial.println("PC可通过WiFi连接后访问: http://" + WiFi.softAPIP().toString());
}

void loop() {
  server.handleClient();
  
  // 检查是否有来自Arduino的响应
  if (ArduinoSerial.available()) {
    String response = ArduinoSerial.readStringUntil('\n');
    response.trim();
    if (response.length() > 0) {
      lastResponse = response;
      Serial.println("Arduino响应: " + response);
    }
  }
  
  delay(10);
}

void setupWebServer() {
  // 根路径 - 返回简单的状态页面
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32指令转发器</title></head><body>";
    html += "<h1>🤖 ESP32指令转发器</h1>";
    html += "<p>状态: <strong>运行正常</strong></p>";
    html += "<p>最后指令: " + lastCommand + "</p>";
    html += "<p>最后响应: " + lastResponse + "</p>";
    html += "<p>请使用PC端网页进行控制</p>";
    html += "</body></html>";
    
    server.send(200, "text/html; charset=utf-8", html);
  });
  
  // 发送指令接口
  server.on("/send", HTTP_POST, []() {
    // 添加CORS头
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (server.hasArg("command")) {
      String command = server.arg("command");
      command.trim();
      
      if (command.length() > 0) {
        Serial.println("接收PC指令: " + command);
        lastCommand = command;
        
        // 发送指令到Arduino
        ArduinoSerial.println(command);
        ArduinoSerial.flush();
        
        // 等待响应
        unsigned long startTime = millis();
        String response = "";
        
        while (millis() - startTime < 2000) { // 等待2秒
          if (ArduinoSerial.available()) {
            String line = ArduinoSerial.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
              response += line + "\n";
            }
          }
          delay(10);
        }
        
        if (response.length() == 0) {
          response = "指令已转发给Arduino，等待响应...";
        }
        
        server.send(200, "text/plain; charset=utf-8", response);
      } else {
        server.send(400, "text/plain; charset=utf-8", "指令不能为空");
      }
    } else {
      server.send(400, "text/plain; charset=utf-8", "缺少指令参数");
    }
  });

  // 批量发送指令接口 - 用于并行执行
  server.on("/batch", HTTP_POST, []() {
    // 添加CORS头
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    if (server.hasArg("commands")) {
      String commands = server.arg("commands");
      commands.trim();
      
      if (commands.length() > 0) {
        Serial.println("接收PC批量指令: " + commands);
        lastCommand = "批量指令: " + commands.substring(0, min(50, (int)commands.length())) + "...";
        
        // 发送批量指令到Arduino（用分号分隔）
        ArduinoSerial.println(commands);
        ArduinoSerial.flush();
        
        // 等待响应
        unsigned long startTime = millis();
        String response = "";
        
        while (millis() - startTime < 3000) { // 等待3秒（批量指令可能需要更长时间）
          if (ArduinoSerial.available()) {
            String line = ArduinoSerial.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
              response += line + "\n";
            }
          }
          delay(10);
        }
        
        if (response.length() == 0) {
          response = "批量指令已转发给Arduino，等待响应...";
        }
        
        server.send(200, "text/plain; charset=utf-8", response);
      } else {
        server.send(400, "text/plain; charset=utf-8", "批量指令不能为空");
      }
    } else {
      server.send(400, "text/plain; charset=utf-8", "缺少批量指令参数");
    }
  });
  
  // 获取状态接口
  server.on("/status", HTTP_GET, []() {
    // 添加CORS头
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    
    String status = "{\"connected\": true, \"lastCommand\": \"" + lastCommand + "\", \"lastResponse\": \"" + lastResponse + "\"}";
    server.send(200, "application/json; charset=utf-8", status);
  });
  
  // 测试连接接口
  server.on("/test", HTTP_GET, []() {
    // 添加CORS头
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    
    server.send(200, "text/plain; charset=utf-8", "ESP32指令转发器运行正常");
  });
  
  // 处理OPTIONS预检请求
  server.on("/send", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200);
  });

  server.on("/batch", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200);
  });
  
  server.on("/status", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.send(200);
  });
  
  server.on("/test", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.send(200);
  });
  
  // 404处理
  server.onNotFound([]() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>页面未找到</title></head><body>";
    html += "<h1>404 - 页面未找到</h1>";
    html += "<p>请求的页面不存在</p>";
    html += "<p><a href='/'>返回首页</a></p>";
    html += "</body></html>";
    
    server.send(404, "text/html; charset=utf-8", html);
  });
} 