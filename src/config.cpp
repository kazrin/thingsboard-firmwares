#include "config.h"

namespace WiFiConfig
{
    const char *ssid = "Your WiFi SSID";
    const char *password = "Your WiFi Password";
}

namespace BasicMQTT
{
    const char *client_id = "Your Client ID. If using access token, it can be any string.";
    const char *user_name = "Your Username. If using access token, it should be an access token.";
    const char *password = "Your Password. If using access token, it can be anything including null.";
}

namespace MQTTBrokerConfig
{
    const char *server = "mqtt.thingsboard.cloud"; // "mqtt.thingsboard.cloud" if using ThingsBoard
    const int port = 1883;
    const int secure_port = 8883;
    const char *topic = "v1/devices/me/telemetry"; // "v1/devices/me/telemetry" if using ThingsBoard
    const char *root_ca = R"(
-----BEGIN CERTIFICATE-----
Your Root CA
-----END CERTIFICATE-----
)";
}
