#pragma once

// ===================== Configuration =====================

// Debugging
const bool enableSerialLogs = true;  // Set to true for debug output

// WiFi Credentials
const char* ssid = "***";
const char* wifiPassword = "***";

// MQTT Configuration
const char* mqttBroker = "***";
const int mqttPort = 1883;
const char* mqttClientID = "SSME";
const char* mqttUsername = "***";
const char* mqttPassword = "***";
const char* mqttTopic = "smartmeter/data";
const char* statusTopic = "smartmeter/status";

// Modbus Configuration
const int slaveID = 254;
const long modbusBaud = 9600;
const int RX_PIN = 16;
const int TX_PIN = 17;
const int SERIAL_CONFIG = SERIAL_8N1;

// OTA Update Configuration
const bool enableOTA = true;
const char* otaHostname = "SmartMeterEmulator";
const char* otaPassword = "SmartMeter2025";  // Strong password for OTA updates
const int otaPort = 8266;                    // Default port for ESP OTA

// Modbus Register Map
enum Registers {
  CONFIG_REG = 63,               // First communication path address (R/W parameter)
  TOTAL_FORWARD_H = 10,          // Current total forward active energy (import) - high word
  TOTAL_FORWARD_L = 11,          // Current total forward active energy (import) - low word
  TOTAL_REVERSE_H = 20,          // Current total reverse active energy (export) - high word
  TOTAL_REVERSE_L = 21,          // Current total reverse active energy (export) - low word
  VOLTAGE_A = 97,                // Voltage of Phase A
  VOLTAGE_B = 98,                // Voltage of Phase B
  VOLTAGE_C = 99,                // Voltage of Phase C
  FREQUENCY_REG = 119,           // Grid frequency
  POWER_A_H = 356,               // Active power of Phase A - high word (W)
  POWER_A_L = 357,               // Active power of Phase A - low word (W)
  POWER_B_H = 358,               // Active power of Phase B - high word (W)
  POWER_B_L = 359,               // Active power of Phase B - low word (W)
  POWER_C_H = 360,               // Active power of Phase C - high word (W)
  POWER_C_L = 361,               // Active power of Phase C - low word (W)
  TOTAL_POWER_H = 362,           // Total active power - high word (W)
  TOTAL_POWER_L = 363,           // Total active power - low word (W)
  DEVICE_TYPE = 20480            // Device type coding - Sungrow DTSU 666 meter (8405)
};