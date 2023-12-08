#include <Arduino.h>
#include <bricks/Camera.hpp>
#include <bricks/SDCard.hpp>
#include <bricks/WiFiCommunication.hpp>

void setup()
{
    Serial.begin(115200);

    // see if the card is present and can be initialized:
    SDCard::initalizeSDCard();
    if (SDCard::writeFile("/tmp.txt", "Hello World!"))
    {
        Serial.println("File written!");
    }

    WiFiCommunication::initalizeWifi();
    Camera::initializeCamera();

    // Move this to within the wifi comms
    WiFiCommunication::server.on("/", HTTP_GET, []
                                 { Camera::serveMjpeg(&WiFiCommunication::server); });

    WiFiCommunication::server.on("/still", HTTP_GET, []
                                 { Camera::serveStill(&WiFiCommunication::server); });

    WiFiCommunication::server.begin();
}

void loop()
{
    WiFiCommunication::server.handleClient();
}