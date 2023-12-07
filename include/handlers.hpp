#include "WifiCam.hpp"
#include <StreamString.h>
#include <uri/UriBraces.h>
#include <SD.h>

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

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("image.jpeg");

    // if the file is available, write to it:
    if (dataFile)
    {
        dataFile.write(frame->data(), frame->size());
        // while (dataFile.available())
        // {
        //     Serial.write(dataFile.read());
        // }
        dataFile.close();
        Serial.println("Saved to SD card!");
    }
    // if the file isn't open, pop up an error:
    else
    {
        Serial.println("error opening datalog.txt");
    }
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
        Serial.printf("changeResolution(%d,%d) failure\n", width, height);
        server.send(500, "text/plain", "changeResolution error\n");
    }
    Serial.printf("changeResolution(%d,%d) success\n", width, height);
    server.on("/", HTTP_GET, []
              { serveMjpeg(); });

    server.on("/still", HTTP_GET, []
              { serveStill(); });
}