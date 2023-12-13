// TEST_SIM7600G_HAT

// Select your modem:
#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define how you're planning to connect to the internet
// These defines are only for this example; they are not needed in other code.
#define TINY_GSM_USE_GPRS true

// Your GPRS credentials, if any
const char apn[] = "id";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Server details
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int port = 80;

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <configs/esp32board.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
HttpClient http(client, server, port);

void setup()
{
    // Set console baud rate
    Serial.begin(115200);
    Serial.println("Wait...");

    // Set GSM module baud rate
    SerialAT.begin(115200);
    SerialAT.setPins(UART_RX, UART_TX);
    delay(3000);

    // Restart takes quite some time
    Serial.println("Initializing modem...");
    while (!modem.init())
    {
    }

    String modemInfo = modem.getModemInfo();
    Serial.print("Modem Info: ");
    Serial.println(modemInfo);
}

void loop()
{
    Serial.print("Waiting for network...");
    if (!modem.waitForNetwork())
    {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" success");

    if (modem.isNetworkConnected())
    {
        Serial.println("Network connected");
    }

#if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    Serial.print(F("Connecting to "));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" success");

    if (modem.isGprsConnected())
    {
        Serial.println("GPRS connected");
    }
#endif

    Serial.print(F("Performing HTTP GET request... "));
    int err = http.get(resource);
    if (err != 0)
    {
        Serial.println(F("failed to connect"));
        delay(10000);
        return;
    }

    int status = http.responseStatusCode();
    Serial.print(F("Response status code: "));
    Serial.println(status);
    if (!status)
    {
        delay(10000);
        return;
    }

    Serial.println(F("Response Headers:"));
    while (http.headerAvailable())
    {
        String headerName = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        Serial.println("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0)
    {
        Serial.print(F("Content length is: "));
        Serial.println(length);
    }
    if (http.isResponseChunked())
    {
        Serial.println(F("The response is chunked"));
    }

    String body = http.responseBody();
    Serial.println(F("Response:"));
    Serial.println(body);

    Serial.print(F("Body length is: "));
    Serial.println(body.length());

    // Shutdown

    http.stop();
    Serial.println(F("Server disconnected"));
#if TINY_GSM_USE_GPRS
    modem.gprsDisconnect();
    Serial.println(F("GPRS disconnected"));
#endif

    // Do nothing forevermore
    while (true)
    {
        delay(1000);
    }
}