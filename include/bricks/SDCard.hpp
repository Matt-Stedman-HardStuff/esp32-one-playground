#pragma once

#include <SD.h>
#include <SPI.h>
#include <configs/esp32_one_config.h>

SPIClass hspi(HSPI);

namespace SDCard
{
    static bool initalizeSDCard()
    {
        hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        if (!SD.begin(SD_CS, hspi, 4000000))
        {
            Serial.println("Card failed, or not present");
            return false;
        }
        else
        {
            Serial.println(SD.cardType());
            Serial.println("card initialized.");
            return true;
        }
    }

    static bool writeFile(String file_name, String file_contents)
    {
        File file = SD.open(file_name, FILE_WRITE, true);
        bool success = file.println(file_contents);
        file.close();
        return success;
    }
}