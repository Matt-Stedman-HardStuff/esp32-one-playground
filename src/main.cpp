// Server details
#define SERVER "vsh.pp.ua"
#include <configs/esp32board.h>
#include <configs/networking.h>
#include <Arduino.h>
#include <bricks/SIMCOM.hpp>

void setup()
{
    Serial.begin(SERIAL_BAUD);
    SIMCOM::init();
}

void loop()
{
    SIMCOM::getFromHTTPServer("/", true);
}