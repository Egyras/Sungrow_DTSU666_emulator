# Smart Meter Modbus Emulator (DTSU666) - ESP32 Version - Not yet tested with real Sungrow Inverter :)

**RS485 Emulator using XY-017 Auto Flow Control Module or similar**

An ESP32-based DTSU666 smart meter emulator with automatic RS485 direction control for industrial IoT applications.

## Features
- ESP32-optimized Modbus RTU implementation
- Automatic TX/RX switching with XY-017 module
- MQTT integration


## Hardware Requirements
- ESP32 development board
- XY-017 TTL to RS485 Converter (Auto Flow Control)
- RS485 bus wiring (A/B lines + GND)
- 3.3V power supply for XY-017 module

## Wiring (ESP32 + XY-017)
| ESP32         | XY-017 Module |
|---------------|---------------|
| GPIO17 (TX2)  | DI            |
| GPIO16 (RX2)  | RO            |
| 3.3V          | VCC           |
| GND           | GND           |

![ESP32 XY-017 Wiring](https://i.imgur.com/esp32_xy017.png)

## Libraries Used
- **ModbusRTU** by [emelianov]
- **PubSubClient** by [Nick O'Leary]
- **ArduinoJSON** by [Benoit Blanchon]
- **ESP32 Arduino Core** (v2.0.0+)

## ESP32 Specific Configuration
```cpp
// settings.h
const int RX_PIN = 16;  // UART2 RX (GPIO16)
const int TX_PIN = 17;  // UART2 TX (GPIO17)

// Modbus Serial Configuration
const long modbusBaud = 9600;
const int SERIAL_CONFIG = SERIAL_8N1;
