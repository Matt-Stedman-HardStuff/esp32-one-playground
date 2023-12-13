#pragma once
#include <configs/esp32board.h>
#include <configs/networking.h>
#define TINY_GSM_MODEM_SIM7600

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
#endif

namespace SIMCOM
{
#define SerialAT Serial1
#ifdef DUMP_AT_COMMANDS
    StreamDebugger debugger(SerialAT, Serial);
    TinyGsm modem(debugger);
#else
    TinyGsm modem(SerialAT);
#endif

    TinyGsmClient client(modem);
#ifndef PORT
#define PORT 80
#endif
    HttpClient http(client, SERVER, PORT);

    struct Header
    {
        String key;
        String value;
    };
    const int MAX_HEADERS = 10;

    struct HttpResponse
    {
        int statusCode = 0;          // HTTP status code
        Header headers[MAX_HEADERS]; // Response headers
        int headerCount = 0;         // Number of headers in response
        String body;                 // Response body
        int contentLength = 0;       // Content length
        bool isChunked = false;      // Flag for chunked response
        String errorMessage;         // Error message, if any

        // Function to add a header (can be used while parsing the response)
        void addHeader(const String &key, const String &value)
        {
            if (headerCount < MAX_HEADERS)
            {
                headers[headerCount].key = key;
                headers[headerCount].value = value;
                headerCount++;
            }
        }

        // Function to check if there was an error
        bool hasError() const { return !errorMessage.isEmpty(); }
    };

    /**
     * @brief Initialize the SIMCOM module against the configs.
     *
     * @return true
     * @return false
     */
    bool init()
    {
        // Set GSM module baud rate
        SerialAT.begin(AT_BAUD);
        SerialAT.setPins(UART_RX, UART_TX);
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        // Restart takes quite some time
        Serial.println("Initializing modem...");
        while (!modem.init())
        {
        }

        String modemInfo = modem.getModemInfo();
        Serial.print("Modem Info: ");
        Serial.println(modemInfo);

        Serial.print("Waiting for network...");
        if (!modem.waitForNetwork())
        {
            Serial.println(" fail");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            return false;
        }
        Serial.println(" success");

        if (modem.isNetworkConnected())
        {
            Serial.println("Network connected");
        }

        // GPRS connection parameters are usually set after network registration
        Serial.print(F("Connecting to "));
        Serial.print(APN_4G);
        if (!modem.gprsConnect(APN_4G))
        {
            Serial.println(" fail");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            return false;
        }
        Serial.println(" success");

        if (modem.isGprsConnected())
        {
            Serial.println("GPRS connected");
        }
        return true;
    }

    HttpResponse postToHTTPServer(String endpoint, String JSON)
    {
        HttpResponse response;
        return response;
    }

    HttpResponse getFromHTTPServer(String endpoint, bool print_to_serial = false)
    {
        HttpResponse response;

        Serial.print(F("Performing HTTP GET request... "));
        int err = http.get(endpoint);
        if (err != 0)
        {
            Serial.println(F("failed to connect"));
            delay(10000);
            return response;
        }

        response.statusCode = http.responseStatusCode();
        for (int i = 0; i < MAX_HEADERS; i++)
        {
            if (http.headerAvailable())
            {
                response.addHeader(http.readHeaderName(), http.readHeaderValue());
                response.headerCount = i;
            }
            else
            {
                break;
            }
        }
        response.contentLength = http.contentLength();
        response.isChunked = http.isResponseChunked();
        response.body = http.responseBody();
        if (print_to_serial)
        {
            Serial.println("Response status code: " + String(response.statusCode));

            Serial.println(F("Response Headers:"));
            for (int i = 0; i < response.headerCount; i++)
            {
                Serial.println("    " + response.headers[i].key + " : " + response.headers[i].value);
            }

            Serial.println("Content length: " + String(response.contentLength));

            Serial.println(F("Response:"));
            Serial.println(response.body);
            Serial.println("The response is " + String(response.isChunked ? "" : "not ") + "chunked.");

            Serial.println("Body length is: " + String(response.body.length()));
        }
        return response;
    }
}