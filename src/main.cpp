#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include "DHT20.h"
#include "Wire.h"
#include <ArduinoOTA.h>
#include <SystemConfig.h>
#include <global.h>
#include <BackgroundTasks.h>

constexpr char WIFI_SSID[] = "ACLAB-IOT";
constexpr char WIFI_PASSWORD[] = "12345678";

constexpr char TOKEN[] = "wuxqwpf6vo0eukpalaut";

constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

constexpr char BLINKING_INTERVAL_ATTR[] = "blinkingInterval";
constexpr char LED_MODE_ATTR[] = "ledMode";
constexpr char LED_STATE_ATTR[] = "ledState";

volatile bool attributesChanged = false;
volatile int ledMode = 0;
volatile bool ledState = false;

constexpr uint16_t BLINKING_INTERVAL_MS_MIN = 10U;
constexpr uint16_t BLINKING_INTERVAL_MS_MAX = 60000U;
volatile uint16_t blinkingInterval = 1000U;

uint32_t previousStateChange;

constexpr int16_t telemetrySendInterval = 10000U;
uint32_t previousDataSend;

constexpr std::array<const char *, 2U> SHARED_ATTRIBUTES_LIST = {
  LED_STATE_ATTR,
  BLINKING_INTERVAL_ATTR
};

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

DHT20 dht20;

RPC_Response setLedSwitchState1(const RPC_Data &data) {
    Serial.println("Received Switch state");
    bool newState = data;
    Serial.print("Switch 1 state change:");
    Serial.println(newState);
    attributesChanged = true;
    digitalWrite(LED1_PIN, newState);
    return RPC_Response("setLedSwitchValue1", newState);
}

RPC_Response setLedSwitchState2(const RPC_Data &data) {
    Serial.println("Received Switch state");
    bool newState = data;
    Serial.print("Switch 1 state change:");
    Serial.println(newState);
    attributesChanged = true;
    digitalWrite(LED2_PIN, newState);
    return RPC_Response("setLedSwitchValue2", newState);
}

RPC_Response setFanState(const RPC_Data &data) {
    Serial.println("Received Switch state");
    int newState = data;
    Serial.print("Fan speed state change:");
    Serial.println(newState);
    attributesChanged = true;
    digitalWrite(FAN_PIN, newState);
    return RPC_Response("setFanState", newState);
}

const std::array<RPC_Callback, 3U> callbacks = {
  RPC_Callback{ "setLedSwitchValue1", setLedSwitchState1},
  RPC_Callback{ "setLedSwitchValue2", setLedSwitchState2},
  RPC_Callback{ "setFanState", setFanState}
};

void processSharedAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0) {
      const uint16_t new_interval = it->value().as<uint16_t>();
      if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX) {
        blinkingInterval = new_interval;
        Serial.print("Blinking interval is set to: ");
        Serial.println(new_interval);
      }
    } else if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0) {
      ledState = it->value().as<bool>();
      digitalWrite(LED1_PIN, ledState);
      Serial.print("LED state is set to: ");
      Serial.println(ledState);
    }
  }
  attributesChanged = true;
}

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been successfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

const bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }
  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void WIFI_TASK(void *pvParameters) 
{
   while (1) {
      if (!reconnect()) {
         return;
      }
      vTaskDelay(10000 / portTICK_PERIOD_MS);
   }
}

void THINGSBOARD_CONNECT_TASK(void *pvParameters) 
{
   while (1) {
      if (!tb.connected()) {
         Serial.print("Connecting to: ");
         Serial.print(THINGSBOARD_SERVER);
         Serial.print(" with token ");
         Serial.println(TOKEN);
         if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
            Serial.println("Failed to connect");
            return;
         }
         tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
         Serial.println("Subscribing for RPC...");
         if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
            Serial.println("Failed to subscribe for RPC");
            return;
         }
         if (!tb.Shared_Attributes_Subscribe(attributes_callback)) {
            Serial.println("Failed to subscribe for shared attribute updates");
            return;
         }
         Serial.println("Subscribe done");
         if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
            Serial.println("Failed to request for shared attributes");
            return;
         }
      }
      tb.loop();
      vTaskDelay(5000 / portTICK_PERIOD_MS);
   }
}

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  //Wire.begin(SDA_PIN,SCL_PIN);
  delay(1000);
  InitWiFi();

  xTaskCreate(WIFI_TASK, "WIFI_TASK", 4096, NULL, 10, NULL);
  xTaskCreate(THINGSBOARD_CONNECT_TASK, "THINGSBOARD_CONNECT_TASK", 4096, NULL, 20, NULL);

  background_Task_setup();  
  Serial.println("SETUP SUCCESSFULLY");
}

void loop() {
  delay(10);
  //Serial.println("HELLO UWUW");
  // if (attributesChanged) {
  //   attributesChanged = false;
  //   tb.sendAttributeData(LED_STATE_ATTR, digitalRead(LED1_PIN));
  // }

  // if (ledMode == 1 && millis() - previousStateChange > blinkingInterval) {
  //   previousStateChange = millis();
  //   digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  //   Serial.print("LED state changed to: ");
  //   Serial.println(!digitalRead(LED_PIN));
  // }

  // if (millis() - previousDataSend > telemetrySendInterval) {
  //   previousDataSend = millis();

    // dht20.read();
    
    // float temperature = dht20.getTemperature();
    // float humidity = dht20.getHumidity();

  //   if (isnan(temperature) || isnan(humidity)) {
  //     Serial.println("Failed to read from DHT20 sensor!");
  //   } else {
  //     Serial.print("Temperature: ");
  //     Serial.print(temperature);
  //     Serial.print(" °C, Humidity: ");
  //     Serial.print(humidity);
  //     Serial.println(" %");

  //     tb.sendTelemetryData("temperature", temperature);
  //     tb.sendTelemetryData("humidity", humidity);
  //   }

  //   tb.sendAttributeData("rssi", WiFi.RSSI());
  //   tb.sendAttributeData("channel", WiFi.channel());
  //   tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
  //   tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
  //   tb.sendAttributeData("ssid", WiFi.SSID().c_str());
  // }

  //tb.loop();
}
