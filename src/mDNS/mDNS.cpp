#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "utils.h"

AsyncWebServer server(80);

void setup()
{
    Serial.begin(115200);

    // WiFi setup
    WiFi.begin(WiFiConfig::ssid, WiFiConfig::password);

    if (!connectToWiFi())
    {
        esp_restart();
    }

    // mDNS setup
    if (MDNS.begin("esp32"))
    {
        Serial.println("mDNS responder started");
    }
    else
    {
        Serial.println("mDNS responder failed");
        esp_restart();
    }

    // HTTP server root setup
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Hello from ESP32!"); });

    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void loop()
{
    // Print IP address
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    delay(10000);
}