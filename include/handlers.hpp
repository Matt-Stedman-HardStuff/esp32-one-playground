#include "WifiCam.hpp"
#include <StreamString.h>
#include <uri/UriBraces.h>

#define width 640
#define height 480

static void serveStill()
{
    auto frame = esp32cam::capture();
    if (frame == nullptr)
    {
        Serial.println("capture() failure");
        server.send(500, "text/plain", "still capture error\n");
        return;
    }
    Serial.printf("capture() success: %dx%d %zub\n", frame->getWidth(), frame->getHeight(),
                  frame->size());

    server.setContentLength(frame->size());
    server.send(200, "image/jpeg");
    WiFiClient client = server.client();
    frame->writeTo(client);
}

static void serveMjpeg()
{
    Serial.println("MJPEG streaming begin");
    WiFiClient client = server.client();
    auto startTime = millis();
    int nFrames = esp32cam::Camera.streamMjpeg(client);
    auto duration = millis() - startTime;
    Serial.printf("MJPEG streaming end: %dfrm %0.2ffps\n", nFrames, 1000.0 * nFrames / duration);
}

void addRequestHandlers()
{
    auto r = esp32cam::Camera.listResolutions().find(width, height);

    if (!esp32cam::Camera.changeResolution(r))
    {
        Serial.printf("changeResolution(%ld,%ld) failure\n", width, height);
        server.send(500, "text/plain", "changeResolution error\n");
    }
    Serial.printf("changeResolution(%ld,%ld) success\n", width, height);
    server.on("/", HTTP_GET, []
              { serveMjpeg(); });

    server.on("/still", HTTP_GET, []
              { serveStill(); });
}