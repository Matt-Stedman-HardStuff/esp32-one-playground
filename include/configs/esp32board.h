#pragma once

#define SERIAL_BAUD 115200

#define ESP32_PICO_TO_RPI
#if defined(ESP32_PICO_TO_RPI)
// UART pins
const int UART_RX = 16;
const int UART_TX = 15;


// UART pins
const int I2C_SDA = 13;
const int I2C_SCL = 14;

// SD Card pins
const int SD_SCK = 14;
const int SD_MISO = 12;
const int SD_MOSI = 13;
const int SD_CS = 15;

// Controllable LED
const int BLINK_LED = 21;

// NO CAMERA!

#elif defined(ESP32_ONE)
// UART pins
const int UART_RX = 3;
const int UART_TX = 1;

// SD Card pins
const int SD_SCK = 14;
const int SD_MISO = 12;
const int SD_MOSI = 13;
const int SD_CS = 15;

// Controllable LED
const int BLINK_LED = 21;

// Camera pins
#include <esp32cam.h>
constexpr esp32cam::Pins CAMERA_PINS{
    D0 : 34,
    D1 : 13,
    D2 : 14,
    D3 : 35,
    D4 : 39,
    D5 : 38,
    D6 : 37,
    D7 : 36,
    XCLK : 4,
    PCLK : 25,
    VSYNC : 5,
    HREF : 27,
    SDA : 18,
    SCL : 23,
    RESET : -1,
    PWDN : -1,
};

#endif
