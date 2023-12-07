#include <Arduino.h>
#include <WiFi.h>
#include <WifiCam.hpp>
#include <handlers.hpp>
#include <configs/esp32_one_config.h>

// #define SPI_MISO_PIN 12
// #define SPI_MOSI_PIN 13
// #define SPI_SCK_PIN 14

#include <SPI.h>
#include <SD.h>

static const char *WIFI_SSID = "EE-Hub-cLM4";
static const char *WIFI_PASS = "flag-DID-ahead";

// esp32cam::Resolution initialResolution;

WebServer server(80);

void setup()
{
    Serial.begin(115200);
    Serial.println();

    // WIFI
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

    // CAMERA
    {
        using namespace esp32cam;

        // initialResolution = Resolution::find(1024, 768);

        Config cfg;
        cfg.setPins(esp32cam::pins::ESP32_One);
        // cfg.setResolution(initialResolution);
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

    // SD CARD
    Serial.print("Initializing SD card...");

    SPI.pin(14, 12, 13);
    // see if the card is present and can be initialized:
    if (!SD.begin(15))
    {
        Serial.println("Card failed, or not present");
        // // don't do anything more:
        // while (1)
        //     ;
    }
    else
    {
        Serial.println("card initialized.");
    }
}

void loop()
{
    server.handleClient();
}