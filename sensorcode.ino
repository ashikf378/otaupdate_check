#ifdef ESP8266
  #include <ESP8266WiFi.h>     /* WiFi library for ESP8266 */
#else
  #include <WiFi.h>            /* WiFi library for ESP32 */
#endif
#include <Wire.h>
#include <PubSubClient.h>
#include "DHT.h"             /* DHT11 sensor library */

#define DHTPIN D1
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define MQ3PIN A0  // Define the analog pin for MQ-3 sensor

#define wifi_ssid "APEX"
#define wifi_password "ashikf378!!"
#define mqtt_server "192.168.0.106"

#define humidity_topic "sensor/DHT11/humidity"
#define temperature_celsius_topic "sensor/DHT11/temperature_celsius"
#define temperature_fahrenheit_topic "sensor/DHT11/temperature_fahrenheit"
#define mq3_topic "sensor/MQ3/ppm"  // Topic to publish MQ-3 sensor data

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(2000);

  // Reading DHT11 sensor data
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Temperature in Celsius: ");
  Serial.println(String(t).c_str());
  client.publish(temperature_celsius_topic, String(t).c_str(), true);

  Serial.print("Temperature in Fahrenheit: ");
  Serial.println(String(f).c_str());
  client.publish(temperature_fahrenheit_topic, String(f).c_str(), true);

  Serial.print("Humidity: ");
  Serial.println(String(h).c_str());
  client.publish(humidity_topic, String(h).c_str(), true);

  // Reading MQ-3 sensor data
  int mq3_value = analogRead(MQ3PIN);
  float mq3_ppm = mq3_value * (5.0 / 1023.0);  // Convert the analog value to voltage and then to ppm

  Serial.print("MQ-3 Sensor PPM: ");
  Serial.println(String(mq3_ppm).c_str());
  client.publish(mq3_topic, String(mq3_ppm).c_str(), true);
}
