#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include <HTTPUpdate.h>

#include "config.h"
#include "utils.h"

const char *FW_TITLE = "TEST";

Preferences preferences;

WiFiClient wifiClient;
WiFiClient OTAclient;
PubSubClient pubsubClient(wifiClient);

const int PIN_NUM = 32;

enum class FW_STATE
{
    UPDATING,
    FINISHED,
    FAILED,
};

void publishFWState(FW_STATE state)
{
    JsonDocument doc;
    String stateStr;
    switch (state)
    {
    case FW_STATE::UPDATING:
        stateStr = "UPDATING";
        break;
    case FW_STATE::FINISHED:
        stateStr = "FINISHED";
        break;
    case FW_STATE::FAILED:
        stateStr = "FAILED";
        break;
    }
    doc["fw_state"] = stateStr;
    String payload;
    serializeJson(doc, payload);
    pubsubClient.publish(MQTTBrokerConfig::topic, payload.c_str());
}

void update_started()
{
    Serial.println("Callback: OTA process started");
    publishFWState(FW_STATE::UPDATING);
}

void update_finished()
{
    Serial.println("Callback: OTA process finished");
    publishFWState(FW_STATE::FINISHED);
}

void update_progress(int cur, int total)
{
    float percentage = (cur * 100.0) / total;
    Serial.printf("Callback: OTA process at %d of %d bytes (%.1f%%)\n", cur, total, percentage);
}

void update_error(int err)
{
    Serial.printf("Callback: OTA fatal error code %d\n", err);
    publishFWState(FW_STATE::FAILED);
}

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

    if (doc["fw_title"] == FW_TITLE && doc["fw_version"] != FW_VERSION)
    {
        Serial.println("Update firmware");

        // Construct update URL
        const String access_token = preferences.getString("access_token");
        String updateUrl = String("http://thingsboard.cloud/api/v1/") +
                           access_token +
                           String("/firmware?title=") +
                           String((const char *)doc["fw_title"]) +
                           String("&version=") +
                           String((const char *)doc["fw_version"]);

        httpUpdate.onStart(update_started);
        httpUpdate.onEnd(update_finished);
        httpUpdate.onProgress(update_progress);
        httpUpdate.onError(update_error);

        t_httpUpdate_return ret = httpUpdate.update(OTAclient, updateUrl.c_str());

        // Output Firmware update result
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
        }
    }
}

void togglePin()
{
    static unsigned long lastToggleTime = 0;
    static bool state = false;

#ifdef BEFORE
    const unsigned long INTERVAL = 1000;
#else
    const unsigned long INTERVAL = 5000;
#endif

    if (millis() - lastToggleTime > INTERVAL)
    {
        digitalWrite(PIN_NUM, state);
        state = !state;
        lastToggleTime = millis();
    }
}

void setup()
{
    Serial.begin(115200);

    // Set Pin mode
    pinMode(PIN_NUM, OUTPUT);

    // Store WiFi config with preferences
    preferences.begin("wifi", false);

#ifdef SETUP_MODE
    if (preferences.getString("ssid") != WiFiConfig::ssid)
    {
        preferences.putString("ssid", WiFiConfig::ssid);
    }
    if (preferences.getString("password") != WiFiConfig::password)
    {
        preferences.putString("password", WiFiConfig::password);
    }
    if (preferences.getString("access_token") != BasicMQTT::user_name)
    {
        preferences.putString("access_token", BasicMQTT::user_name);
    }
#endif
    // Connect to WiFi
    WiFi.begin(
        preferences.getString("ssid"),
        preferences.getString("password"));
    if (!connectToWiFi())
    {
        esp_restart();
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
        const String access_token = preferences.getString("access_token");
        bool connected = pubsubClient.connect(
            BasicMQTT::client_id,
            access_token.c_str(),
            BasicMQTT::password);
        if (connected)
        {
            Serial.println("Connected to MQTT broker");
            pubsubClient.subscribe("v1/devices/me/attributes");
        }
    }

    togglePin();
    delay(1);
}