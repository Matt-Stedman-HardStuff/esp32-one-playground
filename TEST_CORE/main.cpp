// TEST_CORE
#include <Arduino.h>
#include <FastLED.h>
#include <configs/esp32board.h>

void print_task(void *param)
{
    int count = 0;
    for (;;)
    {
        Serial.println("Hello World! " + String(count));
        count += 1;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

CRGB leds[1];
void flash_task(void *param)
{
    FastLED.addLeds<NEOPIXEL, BLINK_LED>(leds, 1); // GRB ordering is assumed
    for (;;)
    {
        for (int i = 0; i < 0xffffff; i += 100)
        {
            leds[0] = i;
            FastLED.show();
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(BLINK_LED, OUTPUT);

    xTaskCreatePinnedToCore(
        print_task,
        "PRINTING",
        4096,
        NULL,
        10,
        NULL,
        0);

    xTaskCreatePinnedToCore(
        flash_task,
        "FLASHING",
        4096,
        NULL,
        10,
        NULL,
        1);
}

void loop()
{
    vTaskDelete(NULL);
}