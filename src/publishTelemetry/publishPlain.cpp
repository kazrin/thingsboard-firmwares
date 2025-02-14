#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "utils.h"

WiFiClient wifiClient;
PubSubClient pubsubClient(wifiClient);

void publish(unsigned long interval = 10000)
{
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish < interval)
    {
        return;
    }
    lastPublish = millis();

    // Generate random values
    JsonDocument doc;
    doc["temperature"] = random(20, 30);
    doc["humidity"] = random(40, 60);
    doc["pressure"] = random(1000, 1010);

    String telemetry;
    serializeJson(doc, telemetry);
    pubsubClient.publish(MQTTBrokerConfig::topic, telemetry.c_str());
    Serial.println("Published with following payload:");
    Serial.println(telemetry);
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(WiFiConfig::ssid, WiFiConfig::password);

    // Connect to WiFi
    if (!connectToWiFi())
    {
        ESP.restart();
    }

    // Set up MQTT client
    pubsubClient.setServer(MQTTBrokerConfig::server, MQTTBrokerConfig::port);
}

void loop()
{
    // Check if MQTT client is connected
    // If connected, publish data. Otherwise, connect to MQTT server
    if (pubsubClient.connected())
    {
        publish();
    }
    else
    {
        pubsubClient.connect(
            BasicMQTT::client_id,
            BasicMQTT::user_name,
            BasicMQTT::password);
    }
}