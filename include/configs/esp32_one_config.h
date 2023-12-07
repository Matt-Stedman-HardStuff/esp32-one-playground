// blinking LED
#define BLINK_LED 21

// #define ESP32CAM_PINS_HPP
/** @brief Camera pins definition. */
#include <WifiCam.hpp>

namespace esp32cam
{
    namespace pins
    {
        constexpr Pins ESP32_One{
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
    }
}