#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ModbusRTU.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "settings.h"  // Configuration header

// ===================== Global Objects =====================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
ModbusRTU modbus;

// ===================== State Management =====================
struct MeterData {
  int32_t total_kwh = 0;         // Total imported energy in kWh 
  int32_t export_kwh = 0;        // Total exported energy in kWh
  int32_t total_power = 0;       // Current total power consumption in W
  int16_t v1 = 230;              // Voltage of Phase A in V
  int16_t v2 = 230;              // Voltage of Phase B in V
  int16_t v3 = 230;              // Voltage of Phase C in V
  int16_t freq = 50;             // Grid frequency in Hz
  int32_t power_a = 0;           // Active power of Phase A in W
  int32_t power_b = 0;           // Active power of Phase B in W
  int32_t power_c = 0;           // Active power of Phase C in W

  bool total_kwh_rx = false;
  bool export_kwh_rx = false;
  bool total_power_rx = false;
  bool v1_rx = false;
  bool v2_rx = false;
  bool v3_rx = false;
  bool freq_rx = false;
  bool power_a_rx = false;
  bool power_b_rx = false;
  bool power_c_rx = false;

  bool allDataReceived() {
    return total_kwh_rx && export_kwh_rx && total_power_rx &&
           v1_rx && v2_rx && v3_rx && freq_rx &&
           power_a_rx && power_b_rx && power_c_rx;
  }
};

MeterData meterData;
bool modbusInitialized = false;
bool registersAdded = false;

// ===================== Modbus Functions =====================
void addRegisterIfNeeded(uint16_t reg, uint16_t value = 0) {
  if(!modbus.Hreg(reg)) {
    modbus.addHreg(reg, value);
    if(enableSerialLogs) Serial.printf("[Modbus] Added register %d\n", reg);
  }
}

void updateModbusRegisters() {
  if(!registersAdded) {
    modbus.addHreg(DEVICE_TYPE, 8405);
    modbus.addHreg(CONFIG_REG, 0);
    registersAdded = true;
    if(enableSerialLogs) Serial.println("[Modbus] Initialized registers");
  }

  modbus.Hreg(CONFIG_REG, 0);

  
  addRegisterIfNeeded(VOLTAGE_A);
  modbus.Hreg(VOLTAGE_A, static_cast<uint16_t>(meterData.v1));
  
  addRegisterIfNeeded(VOLTAGE_B);
  modbus.Hreg(VOLTAGE_B, static_cast<uint16_t>(meterData.v2));
  
  addRegisterIfNeeded(VOLTAGE_C);
  modbus.Hreg(VOLTAGE_C, static_cast<uint16_t>(meterData.v3));

  addRegisterIfNeeded(FREQUENCY_REG);
  modbus.Hreg(FREQUENCY_REG, static_cast<uint16_t>(meterData.freq));

  // total_kwh
  addRegisterIfNeeded(TOTAL_FORWARD_H);
  addRegisterIfNeeded(TOTAL_FORWARD_L);
  uint32_t total_wh = static_cast<uint32_t>(meterData.total_kwh * 100);
  modbus.Hreg(TOTAL_FORWARD_H, (total_wh >> 16) & 0xFFFF);
  modbus.Hreg(TOTAL_FORWARD_L, total_wh & 0xFFFF);

  // export_kwh
  addRegisterIfNeeded(TOTAL_REVERSE_H);
  addRegisterIfNeeded(TOTAL_REVERSE_L);
  uint32_t export_wh = static_cast<uint32_t>(meterData.export_kwh * 100);
  modbus.Hreg(TOTAL_REVERSE_H, (export_wh >> 16) & 0xFFFF);
  modbus.Hreg(TOTAL_REVERSE_L, export_wh & 0xFFFF);

  addRegisterIfNeeded(POWER_A_H);
  addRegisterIfNeeded(POWER_A_L);
  modbus.Hreg(POWER_A_H, (meterData.power_a >> 16) & 0xFFFF);
  modbus.Hreg(POWER_A_L, meterData.power_a & 0xFFFF);

  addRegisterIfNeeded(POWER_B_H);
  addRegisterIfNeeded(POWER_B_L);
  modbus.Hreg(POWER_B_H, (meterData.power_b >> 16) & 0xFFFF);
  modbus.Hreg(POWER_B_L, meterData.power_b & 0xFFFF);

  addRegisterIfNeeded(POWER_C_H);
  addRegisterIfNeeded(POWER_C_L);
  modbus.Hreg(POWER_C_H, (meterData.power_c >> 16) & 0xFFFF);
  modbus.Hreg(POWER_C_L, meterData.power_c & 0xFFFF);

  addRegisterIfNeeded(TOTAL_POWER_H);
  addRegisterIfNeeded(TOTAL_POWER_L);
  modbus.Hreg(TOTAL_POWER_H, (meterData.total_power >> 16) & 0xFFFF);
  modbus.Hreg(TOTAL_POWER_L, meterData.total_power & 0xFFFF);
}

// ===================== Setup Functions =====================
void setupWiFi() {
  if(enableSerialLogs) Serial.printf("[WiFi] Connecting to %s", ssid);
  WiFi.begin(ssid, wifiPassword);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(enableSerialLogs) Serial.print(".");
    if (millis() - startTime > 15000) {
      if(enableSerialLogs) Serial.println("\n[WiFi] Failed to connect!");
      return;
    }
  }
  if(enableSerialLogs) Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void setupOTA() {
  if (!enableOTA) return;
  
  // Port defaults to 3232
  ArduinoOTA.setPort(otaPort);
  
  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(otaHostname);
  
  // No authentication by default
  ArduinoOTA.setPassword(otaPassword);
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    if(enableSerialLogs) Serial.println("[OTA] Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    if(enableSerialLogs) Serial.println("\n[OTA] Update complete");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if(enableSerialLogs) Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    if(enableSerialLogs) {
      Serial.printf("[OTA] Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    }
  });
  
  ArduinoOTA.begin();
  if(enableSerialLogs) Serial.println("[OTA] OTA service initialized");
}

bool connectMQTT() {
  if (mqttClient.connected()) return true;

  if(enableSerialLogs) Serial.printf("[MQTT] Connecting to %s...", mqttBroker);
  
  bool connected = mqttClient.connect(mqttClientID, mqttUsername, mqttPassword, 
                                    statusTopic, 1, true, "offline");
  if (connected) {
    if(enableSerialLogs) {
      Serial.println("connected!");
      Serial.printf("[MQTT] Subscribed to: %s\n", mqttTopic);
    }
    mqttClient.publish(statusTopic, "online", true);
    mqttClient.subscribe(mqttTopic);
  } else if (enableSerialLogs) {
    Serial.printf("failed! State: %d\n", mqttClient.state());
  }
  return connected;
}

// ===================== MQTT Callback =====================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if(enableSerialLogs) {
    Serial.printf("\n[MQTT] Message received on topic: %s\n", topic);
    Serial.printf("[MQTT] Payload length: %d bytes\n", length);
  }

  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, payload, length);

  if (err && enableSerialLogs) {
    Serial.printf("[MQTT] JSON error: %s\n", err.c_str());
    return;
  }

  if (!doc.containsKey("SML")) {
    if(enableSerialLogs) Serial.println("[MQTT] Missing 'SML' object!");
    return;
  }

  JsonObject sml = doc["SML"];
  bool updated = false;

  #define UPDATE_FIELD(field, var, flag) \
    if (sml.containsKey(#field)) { \
      meterData.var = sml[#field]; \
      if (!meterData.flag) meterData.flag = true; \
      updated = true; \
      if(enableSerialLogs) { \
        Serial.printf("- %s: ", #field); \
        Serial.printf("%d\n", meterData.var); \
      } \
    }

  if(enableSerialLogs) Serial.println("[MQTT] Updated fields:");
  UPDATE_FIELD(total_kwh, total_kwh, total_kwh_rx)
  UPDATE_FIELD(export_total_kwh, export_kwh, export_kwh_rx)
  UPDATE_FIELD(Power_curr, total_power, total_power_rx)
  UPDATE_FIELD(volt_p1, v1, v1_rx)
  UPDATE_FIELD(volt_p2, v2, v2_rx)
  UPDATE_FIELD(volt_p3, v3, v3_rx)
  UPDATE_FIELD(freq, freq, freq_rx)
  UPDATE_FIELD(Power_A, power_a, power_a_rx)
  UPDATE_FIELD(Power_B, power_b, power_b_rx)
  UPDATE_FIELD(Power_C, power_c, power_c_rx)

  if(enableSerialLogs && !updated) Serial.println("(None)");

  if(modbusInitialized) updateModbusRegisters();
  if (!modbusInitialized && meterData.allDataReceived()) {
    Serial2.begin(modbusBaud, SERIAL_CONFIG, RX_PIN, TX_PIN);
    modbus.begin(&Serial2);
    modbus.slave(slaveID);
    modbusInitialized = true;
    if(enableSerialLogs) Serial.println("\n[Modbus] Server started!");
    updateModbusRegisters();
  }
}

// ===================== Main Program =====================
void setup() {
  if(enableSerialLogs) {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n===== Smart Meter Emulator =====");
  }
  
  setupWiFi();
  
  if (enableOTA && WiFi.status() == WL_CONNECTED) {
    setupOTA();
  }
  
  mqttClient.setServer(mqttBroker, mqttPort);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  static unsigned long lastReconnectAttempt = 0;
  static unsigned long lastReport = 0;
  static unsigned long lastModbusUpdate = 0;

  // Handle OTA updates
  if (enableOTA) {
    ArduinoOTA.handle();
  }

  // Handle Modbus with priority
  if(modbusInitialized) {
    modbus.task();
  }

  // Non-blocking MQTT management
  if (!mqttClient.connected()) {
    if (millis() - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = millis();
      if (connectMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqttClient.loop();
  }

  // Debug reporting
  if (enableSerialLogs && millis() - lastReport > 5000) {
    if(modbusInitialized) {
      Serial.println("[SYSTEM] Modbus operational");
    } else {
      Serial.printf("[SYSTEM] Waiting for data: %d/10 fields received\n", 
        (meterData.total_kwh_rx + meterData.export_kwh_rx + meterData.total_power_rx +
         meterData.v1_rx + meterData.v2_rx + meterData.v3_rx + meterData.freq_rx +
         meterData.power_a_rx + meterData.power_b_rx + meterData.power_c_rx));
    }
    
    // Report OTA status
    if (enableOTA) {
      Serial.printf("[SYSTEM] OTA update service active on %s.local:%d\n", 
                   otaHostname, otaPort);
    }
    
    lastReport = millis();
  }

  // Yield to system
  delay(1);
}