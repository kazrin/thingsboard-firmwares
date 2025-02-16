#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "utils.h"

class PinState
{
private:
    bool state;
    int pinNumber;
    String pinNumberStr;

public:
    PinState(int pinNumber) : pinNumber(pinNumber),
                              pinNumberStr(String(pinNumber))
    {
        state = false;
        pinMode(pinNumber, OUTPUT);
    }

    String stringify()
    {
        JsonDocument doc;
        doc["pinState"] = state;
        String output;
        serializeJson(doc, output);
        return output;
    }

    void toggle()
    {
        digitalWrite(pinNumber, !state);
        state = !state;
    }

    void setState(bool state)
    {
        digitalWrite(pinNumber, state);
        this->state = state;
    }
};

PinState pinState(32);

WiFiClient wifiClient;
PubSubClient pubsubClient(wifiClient);

void callback(char *topic, byte *payload, unsigned int length)
{
    String incoming = "";
    for (int i = 0; i < length; i++)
    {
        incoming += (char)payload[i];
    }
    Serial.println("Message arrived [" + String(topic) + "] " + incoming);

    JsonDocument doc;
    deserializeJson(doc, incoming);

    if (doc["method"] == "toggle")
    {
        pinState.toggle();
        String response = pinState.stringify();
        pubsubClient.publish("v1/devices/me/attributes", response.c_str());
    }
}

void setup()
{
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(WiFiConfig::ssid, WiFiConfig::password);
    if (!connectToWiFi())
    {
        ESP.restart();
    }

    pubsubClient.setServer(MQTTBrokerConfig::server, MQTTBrokerConfig::port);
    pubsubClient.setCallback(callback);
}

void loop()
{
    if (pubsubClient.connected())
    {
        pubsubClient.loop();
    }
    else
    {
        Serial.println("Connecting to MQTT broker...");
        bool connected = pubsubClient.connect(
            BasicMQTT::client_id,
            BasicMQTT::user_name,
            BasicMQTT::password);
        if (connected)
        {
            pubsubClient.subscribe("v1/devices/me/rpc/request/+");
        }
    }

    delay(1);
}