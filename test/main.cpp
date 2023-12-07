#include <Arduino.h>
#include <configs/esp32_one_config.h>

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

void flash_task(void *param)
{
    for (;;)
    {
        for (int i = 0; i < 4; i++)
        {
            digitalWrite(BLINK_LED, 0);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            digitalWrite(BLINK_LED, 1);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
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
        1024,
        NULL,
        10,
        NULL,
        1);
}

void loop()
{
    vTaskDelete(NULL);
}