# ESP32 IP Address Guide

## 📍 IP Address `192.168.4.1` Description

### Question: Is this IP address always fixed?

**Answer: Usually fixed, but it is recommended to confirm the actual IP address via serial monitor.**

## 🔍 Detailed Description

### 1. IP Address Nature

- **`192.168.4.1`** is the **default gateway IP address** of ESP32 in **hotspot (softAP) mode**
- This is **ESP32 device's own IP address**, not your computer's IP address
- This IP address is automatically assigned by ESP32 library, usually fixed as `192.168.4.1`

### 2. Who Can Access This IP?

**Any device connected to ESP32 hotspot can access this IP address:**
- ✅ Your computer (after connecting to `RobotControl` WiFi)
- ✅ Other people's computers (after connecting to `RobotControl` WiFi)
- ✅ Phones, tablets, and other devices (after connecting to `RobotControl` WiFi)

### 3. How to Confirm Actual IP Address?

ESP32 will print the actual IP address in serial monitor when starting:

```
ESP32 WiFi Command Forwarder Starting...
WiFi Hotspot Created: RobotControl
Password: 12345678
IP Address: 192.168.4.1          ← Actual IP displayed here
PC can access via WiFi: http://192.168.4.1
```

**Viewing Method:**
1. Open Arduino IDE serial monitor
2. Set baud rate to 115200
3. Restart ESP32
4. Check "IP Address" information in serial output

### 4. Will IP Address Change?

**Usually does not change:**
- ESP32 library default behavior is to use `192.168.4.1` as softAP gateway IP
- Unless ESP32 library version update changes default behavior, IP address is usually fixed

**But to be safe:**
- Recommend confirming IP address via serial monitor each startup
- If IP address is different, use the actually displayed IP address

### 5. How to Set Fixed IP Address?

If you need to ensure IP address is fixed as `192.168.4.1`, you can manually configure in code:

In `ESP32/ESP32_WiFi_Server.ino`:

```cpp
// IP address configuration (uncomment to use fixed IP)
IPAddress local_IP(192, 168, 4, 1);     // ESP32 IP address
IPAddress gateway(192, 168, 4, 1);        // Gateway address (usually same as IP)
IPAddress subnet(255, 255, 255, 0);      // Subnet mask

void setup() {
  // ...
  
  // Configure fixed IP address
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("IP configuration failed, using default IP");
  }
  
  WiFi.softAP(ssid, password);
  // ...
}
```

### 6. Common Questions

**Q: Why can't I connect when accessing `192.168.4.1`?**

A: Possible reasons:
1. Not connected to ESP32 WiFi hotspot `RobotControl`
2. ESP32 IP address is not `192.168.4.1` (check serial monitor to confirm)
3. ESP32 did not start normally
4. Firewall or security software blocking connection

**Solutions:**
1. Confirm connected to `RobotControl` WiFi (password: 12345678)
2. Check serial monitor to confirm ESP32's actual IP address
3. Use actual IP address to access

**Q: What is my computer's IP address?**

A: After connecting to ESP32 hotspot, your computer will get an IP address, usually:
- `192.168.4.2` or `192.168.4.3`, etc. (automatically assigned)
- This IP address is **your computer's IP**, not ESP32's IP
- When accessing ESP32, should use ESP32's IP (`192.168.4.1`), not your computer's IP

**Q: Can it be used on different computers?**

A: Yes! As long as:
1. Computer connects to ESP32 WiFi hotspot `RobotControl`
2. Use ESP32's IP address (usually `192.168.4.1`) to access
3. Any device can connect and use simultaneously

## 📝 Summary

- ✅ `192.168.4.1` is ESP32's IP address, usually fixed
- ✅ Any device connected to ESP32 hotspot can access
- ✅ Recommend confirming actual IP address via serial monitor
- ✅ If IP address is different, use the actually displayed IP address

---

**Last Updated**: 2024

