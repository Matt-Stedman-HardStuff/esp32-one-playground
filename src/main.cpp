#include <Arduino.h>
#include <WiFi.h>
#include <WifiCam.hpp>
#include <handlers.hpp>
#include <configs/esp32_one_config.h>

static const char *WIFI_SSID = "EE-Hub-cLM4";
static const char *WIFI_PASS = "flag-DID-ahead";

esp32cam::Resolution initialResolution;

WebServer server(80);

void setup()
{
    Serial.begin(115200);
    Serial.println();

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

    {
        using namespace esp32cam;

        initialResolution = Resolution::find(1024, 768);

        Config cfg;
        cfg.setPins(esp32cam::pins::ESP32_One);
        cfg.setResolution(initialResolution);
        cfg.setJpeg(80);

        bool ok = Camera.begin(cfg);
        if (!ok)
        {
            Serial.println("camera initialize failure");
            delay(5000);
            ESP.restart();
        }
        Serial.println("camera initialize success");
    }

    Serial.println("camera starting");
    Serial.print("http://");
    Serial.println(WiFi.localIP());

    addRequestHandlers();
    server.begin();
}

void loop()
{
    server.handleClient();
}