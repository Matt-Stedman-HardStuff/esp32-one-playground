#pragma once
#include <configs/esp32board.h>
#include <configs/networking.h>

#define TINY_GSM_MODEM_SIM7600

#include <SSLClientESP32.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <TimeLib.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
#endif

namespace SIMCOM
{

#pragma region DEFINES
#define SerialAT Serial1
#ifdef DUMP_AT_COMMANDS
    StreamDebugger debugger(SerialAT, Serial);
    TinyGsm modem(debugger);
#else
    TinyGsm modem(SerialAT);
#endif
#include <configs/certificate.h>
    // #include <configs/trust_anchors.h>
    TinyGsmClient base_client(modem);
    SSLClientESP32 client(&base_client); //, TAs, (size_t)2, GPIO_NUM_11);
    HttpClient http(client, SERVER, PORT);

#ifndef PORT
#define PORT 80
#endif

#pragma endregion

#pragma region HTTP_STRUCTS
    struct Header
    {
        String key;
        String value;
    };
    const int MAX_HEADERS = 10;

    struct HttpRequest
    {
        Header headers[MAX_HEADERS]; // Response headersI
        int headerCount = 0;         // Number of headers in response
        String content = "";         // Request content

        // Function to add a header (can be used while parsing the response)
        void addHeader(const String &key, const String &value)
        {
            if (headerCount < MAX_HEADERS)
            {
                headers[headerCount].key = key;
                headers[headerCount].value = value;
                headerCount++;
            }
            else
                Serial.println("MAX HEADERS REACHED!");
        }

        /**
         * @brief Clear the contents of the HTTP request
         *
         * @param clear_all
         */
        void clear(bool ignore_headers = false)
        {
            content = ""; // Clear the content

            if (!ignore_headers)
            {
                // Reset headers and header count
                for (int i = 0; i < headerCount; ++i)
                {
                    headers[i].key = "";
                    headers[i].value = "";
                }
                headerCount = 0;
            }
        }

        void print() const
        {
            Serial.println("Headers:");
            for (int i = 0; i < headerCount; i++)
            {
                Serial.println(headers[i].key + " : " + headers[i].value);
            }
            Serial.println("Content:");
            Serial.println(content);
        }
    };

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
            else
                Serial.println("MAX HEADERS REACHED!");
        }

        // Function to check if there was an error
        bool hasError() const { return !errorMessage.isEmpty(); }
        bool success() const { return statusCode >= 200 && statusCode < 300; }
        void print() const
        {
            Serial.println("Response status code: " + String(statusCode));
            Serial.println(F("Response Headers:"));
            for (int i = 0; i < headerCount; i++)
                Serial.println("    " + headers[i].key + " : " + headers[i].value);
            Serial.println("Content length: " + String(contentLength));
            Serial.println(F("Response:"));
            Serial.println(body);
            Serial.println("Response is " + String(isChunked ? "" : "not ") + "chunked.");
            Serial.println("Body length is: " + String(body.length()));
        }
    };
#pragma endregion

#pragma region GPS_STRUCTS
#define GPS_ACCURACY_LIMIT 30000 // 30 metres
    struct GPSResponse
    {
        float lat = 0;
        float lon = 0;
        float speed = 0;
        float alt = 0;
        int vsat = 0;
        int usat = 0;
        float accuracy = 0;
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int min = 0;
        int sec = 0;

        bool withinAccuracy() const { return (accuracy <= GPS_ACCURACY_LIMIT && accuracy != 0); }
        void print()
        {
            Serial.println("Latitude:" + String(lat, 8) + "\tLongitude:" + String(lon, 8));
            Serial.println("Speed:" + String(speed) + "\tAltitude:" + String(alt));
            Serial.println("Visible Satellites:" + String(vsat) + "\tUsed Satellites:" + String(usat));
            Serial.println("Accuracy:" + String(accuracy) + ", that is " + String(withinAccuracy() ? "" : "not ") + "within tolerance!");
            Serial.println("Year:" + String(year) + "\tMonth:" + String(month) + "\tDay:" + String(day));
            Serial.println("Hour:" + String(hour) + "\tMinute:" + String(min) + "\tSecond:" + String(sec));
        }
    };
#pragma endregion

    /**
     * @brief Get the GPS coordinates as a GPS response
     *
     * @param print_response_to_serial Optionally print the response to serial
     * @return GPSResponse
     */
    GPSResponse getGPSCoordinates()
    {
        GPSResponse gps_response;
        modem.getGPS(&gps_response.lat, &gps_response.lon, &gps_response.speed, &gps_response.alt, &gps_response.vsat, &gps_response.usat, &gps_response.accuracy,
                     &gps_response.year, &gps_response.month, &gps_response.day, &gps_response.hour, &gps_response.min, &gps_response.sec);
        return gps_response;
    }

    /**
     * @brief Get the time from the network cell tower
     * @returns true if successful, otherwise false
     */
    bool updateTime()
    {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int min = 0;
        int sec = 0;
        if (modem.getGPSTime(&year, &month, &day, &hour, &min, &sec))
        {
            if (year == 2023)
            {
                setTime(hour, min, sec, day, month, year);

                Serial.println("Time set from GPS");
                return true;
            }
        }

        float timezone;
        if (modem.getNetworkTime(&year, &month, &day, &hour, &min, &sec, &timezone))
        {
            if (year == 2023)
            {
                setTime(hour, min, sec, day, month, year);
                Serial.println("Time set from GSM Network");
                return true;
            }
        }
        return false;
    }

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

        delay(3000);

        // Restart takes quite some time
        Serial.println("Initializing modem... ");
        while (!modem.init())
        {
        }

        String modemInfo = modem.getModemInfo();
        Serial.print("Modem Info: ");
        Serial.println(modemInfo);

        Serial.print("Waiting for network... ");
        if (!modem.waitForNetwork())
        {
            Serial.println(" fail");
            return false;
        }

        if (modem.isNetworkConnected())
        {
            Serial.println("Network connected");
        }

        // Enable full functionality
        modem.setPhoneFunctionality(1);

        // Turn on the GPS
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

        // Turn on the GPS
        modem.disableGPS();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (modem.enableGPS())
        {
            Serial.println("GPS enabled");
        }

        // Us the GPS to update the time
        Serial.print("Setting the RTC... ");
        while (1)
        {
            if (updateTime())
            {
                time_t now_time = now();
                break;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        // http.connectionKeepAlive();
        client.setCACert(root_ca);
        client.setTimeout(5000);
        client.setHandshakeTimeout(5000);
        return true;
    }

    /**
     * @brief Post some content string to a given endpoint of the attached server
     *
     * @param endpoint Given endpoint
     * @param content String of content to post
     * @param print_response_to_serial
     * @return HttpResponse
     */
    HttpResponse postToHTTPServer(String endpoint, HttpRequest *request)
    {
        HttpResponse response;
        // Refresh the SSL client to give us a bit of breathing room
        // client.flush();
        // http.flush();
        vTaskDelay(250 / portTICK_PERIOD_MS);
        // Post the HTTP request
        http.beginRequest();
        http.post(endpoint);

        for (int i = 0; i < request->headerCount; i++)
        {
            http.sendHeader(request->headers[i].key, request->headers[i].value);
        }
        http.sendHeader("Content-Length", request->content.length());
        http.beginBody();
        // http.println(request->content);
        http.print(request->content);
        http.endRequest();

        if (client.getWriteError() != 0)
        {
            response.statusCode = -1;
            http.stop();
            return response;
        }

        Serial.print(F("Performing HTTP POST request... "));

        response.statusCode = http.responseStatusCode();
        if (response.success())
        {
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
            response.body = http.responseBody();
        }
        else
        {
            http.skipResponseHeaders();
            http.responseBody();
        }
        http.stop();
        return response;
    }

    HttpResponse getFromHTTPServer(String endpoint, HttpRequest *request = nullptr)
    {
        HttpResponse response;
        // Refresh the SSL client to give us a bit of breathing room
        // http.flush();
        vTaskDelay(250 / portTICK_PERIOD_MS);
        // Get
        http.beginRequest();
        http.get(endpoint);

        if (client.getWriteError() != 0)
        {
            response.statusCode = -1;
            http.stop();
            return response;
        }
        if (request != nullptr)
        {
            for (int i = 0; i < request->headerCount; i++)
            {
                http.sendHeader(request->headers[i].key, request->headers[i].value);
            }
            if (request->content.length())
            {
                http.sendHeader("Content-Length", request->content.length());
                http.beginBody();
                http.println(request->content);
            }
        }
        http.endRequest();
        Serial.print(F("Performing HTTP GET request... "));

        response.statusCode = http.responseStatusCode();
        if (response.success())
        {
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
        }
        else
        {
            http.skipResponseHeaders();
            http.responseBody();
        }
        http.stop();
        return response;
    }

    String formatTimeISO8601(time_t t)
    {
        char buffer[25];

        // Break down time_t into its components
        int Year = year(t);
        int Month = month(t);
        int Day = day(t);
        int Hour = hour(t);
        int Minute = minute(t);
        int Second = second(t);

        // Format the string in ISO 8601 format
        // Note: This assumes UTC time. Adjust accordingly if using local time.
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
                 Year, Month, Day, Hour, Minute, Second);

        return String(buffer);
    }

    time_t formatTimeFromISO8601(String timestamp)
    {
        int Year, Month, Day, Hour, Minute, Second;
        sscanf(timestamp.c_str(), "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
               &Year, &Month, &Day, &Hour, &Minute, &Second);
        tmElements_t tm;
        tm.Year = Year - 1970; // Adjust year
        tm.Month = Month;      // Adjust month
        tm.Day = Day;
        tm.Hour = Hour;
        tm.Minute = Minute;
        tm.Second = Second;
        return makeTime(tm); // Convert to time_t
    }

}