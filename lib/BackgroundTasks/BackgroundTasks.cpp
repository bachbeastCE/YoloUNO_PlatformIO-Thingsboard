#include "BackgroundTasks.h"
#include "SystemConfig.h"
#include "global.h"
#include "button.h"

#include <Arduino.h>
#include <WiFi.h>
#include <DHT20.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <mqtt_client.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>


void READ_DHT_TASK(void *pvParameters) 
{
   //Wire.begin(SDA_PIN, SCL_PIN);
   //DHT20 dht20;
   //dht20.begin();


   DHT dht11(DHT11_SIGNAL_PIN, DHT11);
   dht11.begin();

   while (1) {
      temperature = dht11.readTemperature();
      humidity = dht11.readHumidity();

      if (isnan(temperature) || isnan(humidity)) {
         Serial.println("Failed to read from DHT20 sensor!");
      } 
      else 
      {
         Serial.print("Temperature: ");
         Serial.print(temperature);
         Serial.print(" °C, Humidity: ");
         Serial.print(humidity);
         Serial.println(" %");

         tb.sendTelemetryData("temperature", temperature);
         tb.sendTelemetryData("humidity", humidity);
      }

      // tb.sendAttributeData("rssi", WiFi.RSSI());
      // tb.sendAttributeData("channel", WiFi.channel());
      // tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
      // tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
      // tb.sendAttributeData("ssid", WiFi.SSID().c_str());

      vTaskDelay(20000 / portTICK_PERIOD_MS);
   }
}

void READ_BUTTON_TASK(void *pvParameters) 
{
   pinMode(button1, INPUT_PULLUP);
   pinMode(button2, INPUT_PULLUP);
   while (1) {
      getKeyInput();
      vTaskDelay(10 / portTICK_PERIOD_MS);
   }
}

void READ_LED_TASK(void *pvParameters) 
{
   pinMode(LED1_PIN, OUTPUT);
   pinMode(LED2_PIN, OUTPUT);
   
   while (1) {
      if (isButtonPressed(0)) {
         led1_state = !led1_state;   
         if(led1_state) {
            digitalWrite(LED1_PIN, HIGH);
         } 
         else 
         {
            digitalWrite(LED1_PIN, LOW);
         }  
      }

      if(isButtonPressed(1)) {
         led2_state = !led2_state;
         if(led2_state) {
            digitalWrite(LED2_PIN, HIGH);
         } 
         else 
         {
            digitalWrite(LED2_PIN, LOW);
         }
         RPC_Response("setLedSwitchValue2", led2_state);
      }
      vTaskDelay(300 / portTICK_PERIOD_MS);
   }
}

void FAN_TASK(void *pvParameters) 
{
   pinMode(FAN_PIN, OUTPUT);
   while (1) {
      if (isButtonPressed(2)) {;
         if(fan_state == 0) {
            fan_state =1;
            digitalWrite(FAN_PIN, HIGH);
         } 
         else 
         {
            fan_state =0;
            digitalWrite(FAN_PIN, LOW);
         }
      }
      vTaskDelay(300 / portTICK_PERIOD_MS);
   }
}

void MQ_TASK(void *pvParameters)
{
   // Initialize the MQ2 sensor
   MQUnifiedsensor MQ2("Arduino", 5, 10, 17, "MQ2");

   // Set the calibration values for the MQ2 sensor
   MQ2.setRegressionMethod(1); // Set the regression method to linear
   MQ2.setA(605.18); // Set the 'a' value for the MQ2 sensor
   MQ2.setB(-3.937); // Set the 'b' value for the MQ2 sensor

   // Calibrate the sensor
   MQ2.init();
   Serial.print("Calibrating please wait.");
   float calcR0 = 0;
   for(int i = 1; i<=10; i++)
   {
      MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
      calcR0 += MQ2.calibrate(9.83);
      Serial.print(".");
   }
   MQ2.setR0(calcR0/10);
   Serial.println("  done!.");
   if(isinf(calcR0)) {Serial.println("Warning: Connection issue or bad calibration");}

   while(1){
      MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
      float ppm = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
      Serial.print("PPM: ");
      Serial.println(ppm); // Print the PPM value to the serial port
      tb.sendTelemetryData("airquality", ppm);
      // Add your logic to process the PPM value here

      vTaskDelay(10000 / portTICK_PERIOD_MS); // Sampling frequency
  }
}

void background_Task_setup() 
{
   xTaskCreate(READ_DHT_TASK, "READ_DHT_TASK", 4096, NULL, 10, NULL);
   delay(25);
   xTaskCreate(READ_BUTTON_TASK, "READ_BUTTON_TASK", 4096, NULL, 10, NULL);
   delay(25);
   xTaskCreate(READ_LED_TASK, "READ_LED_TASK", 4096, NULL, 10, NULL);
   delay(25);
   xTaskCreate(FAN_TASK, "FAN_TASK", 4096, NULL, 10, NULL);
   delay(25);
   xTaskCreate(MQ_TASK, "MQ_TASK", 4096, NULL, 10, NULL);
}

 
