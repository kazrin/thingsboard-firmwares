#include <WiFi.h>
#include "utils.h"

bool connectToWiFi(unsigned long timeout)
{
    unsigned long startTime = millis();
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime > timeout)
        {
            Serial.println("\nWiFi connection failed. Restarting...");
            return false;
        }
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi");
    return true;
}
