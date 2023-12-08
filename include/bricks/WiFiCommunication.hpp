#pragma once

#include <WebServer.h>
#include <configs/networking.h>
#include <WiFi.h>

namespace WiFiCommunication
{
    WebServer server(80);

    void initalizeWifi()
    {
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.println("WiFi failure");
            delay(5000);
            ESP.restart();
        }
        Serial.println("WiFi connected");

        Serial.print("http://");
        Serial.println(WiFi.localIP());
    }
}