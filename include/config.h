#pragma once

namespace WiFiConfig
{
    extern const char *ssid;
    extern const char *password;
}

namespace AccessToken
{
    extern const char *access_token;
}

namespace BasicMQTT
{
    extern const char *client_id;
    extern const char *user_name;
    extern const char *password;
}

namespace MQTTBrokerConfig
{
    extern const char *server;
    extern const int port;
    extern const int secure_port;
    extern const char *topic;
    extern const char *root_ca;
}