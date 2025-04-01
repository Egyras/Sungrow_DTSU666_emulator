# Sungrow Smart Meter Modbus Emulator (DTSU666) - ESP32 Version - Not yet tested with real Sungrow Inverter :)

**RS485 Emulator using XY-017 Auto Flow Control Module or similar**

An ESP32-based DTSU666 smart meter emulator with automatic RS485 direction control for industrial IoT applications.

## Features
- ESP32-optimized Modbus RTU implementation
- Automatic TX/RX switching with XY-017 module
- MQTT integration
- OTA update option


## Hardware Requirements
- ESP32 development board
- XY-017 TTL to RS485 Converter (Auto Flow Control)
- RS485 bus wiring (A/B lines + GND)
- 3.3V power supply for XY-017 module

## Wiring (ESP32 + XY-017)
| ESP32         | XY-017 Module |
|---------------|---------------|
| GPIO17 (TX2)  | TXD            |
| GPIO16 (RX2)  | RXD            |
| 3.3V          | VCC           |
| GND           | GND           |


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
```
## Installation
1. Clone this repository
2. Install required libraries through Arduino Library Manager
3. Configure `settings.h` with your network and MQTT credentials
4. Upload to ESP32
5. Connect RS485 module to TTL pins (default: GPIO16-RX, GPIO17-TX)

## Usage
1. Power up the device
2. Monitor serial port (115200 baud) for initialization status
3. Verify MQTT connection in your broker logs
4. Test Modbus communication using any Modbus RTU client

**MQTT Topics:**
- `smartmeter/data` - Meter data in JSON format
- `smartmeter/status` - Device status (online/offline)

**Sample JSON Payload:**
```json
{
  "SML": {
    "total_kwh": 1234,
    "export_total_kwh": 789,
    "Power_curr": 1500,
    "volt_p1": 230,
    "volt_p2": 231,
    "volt_p3": 229,
    "freq": 50,
    "Power_A": 500,
    "Power_B": 600,
    "Power_C": 400
  }
}
```
## Related Projects

For alternative implementations and community-driven approaches to Sungrow meter communication, you might want to explore:

- [Sungrow-Meter-cheater](https://github.com/Linux-RISC/Sungrow-Meter-cheater)
