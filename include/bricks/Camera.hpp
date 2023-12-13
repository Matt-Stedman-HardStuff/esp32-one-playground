#pragma once

/*
    Utilize the CAMERA module. You can serve images and video streams up to a client.
    The camera is very pin-hungry, so you are limited with what other functionality you can use with it.
    Assume HSPI and default i2c are off limits (no SD card 😡).
*/

#include "WifiCam.hpp"
#include <StreamString.h>
#include <uri/UriBraces.h>
#include <esp32cam.h>
#include <configs/esp32board.h>

#define width 640
#define height 480
int file_num = 0;

namespace Camera
{
    esp32cam::Resolution initialResolution;
    esp32cam::Config cfg;

    static bool initializeCamera()
    {
        initialResolution = esp32cam::Resolution::find(1024, 768);

        cfg.setPins(CAMERA_PINS);
        cfg.setResolution(initialResolution);
        cfg.setJpeg(80);
        if (esp32cam::Camera.begin(cfg))
        {
            Serial.println("Camera initialize success.");
            return true;
        }
        else
        {
            Serial.println("Camera failed to initialise!");
            return true;
        }
    }

    static void serveStill(WebServer *server)
    {
        auto frame = esp32cam::capture();
        delay(100);

        if (frame == nullptr)
        {
            Serial.println("Still capture error...");
            server->send(500, "text/plain", "Still capture error...\n");
            return;
        }
        Serial.printf("capture() success: %dx%d %zub\n", frame->getWidth(), frame->getHeight(),
                      frame->size());

        server->setContentLength(frame->size());
        server->send(200, "image/jpeg");

        WiFiClient client = server->client();
        frame->writeTo(client);
    }

    static void serveMjpeg(WebServer *server)
    {
        Serial.println("MJPEG streaming begin");
        WiFiClient client = server->client();
        auto startTime = millis();
        int nFrames = esp32cam::Camera.streamMjpeg(client);
        auto duration = millis() - startTime;
        Serial.printf("MJPEG streaming end: %dfrm %0.2ffps\n", nFrames, 1000.0 * nFrames / duration);
    }
}