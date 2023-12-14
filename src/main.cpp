#include <Arduino.h>
#define _DISABLE_TLS_

// Server details
// Configs
#include <configs/esp32board.h>
#include <configs/networking.h>

// Bricks
#include <bricks/SIMCOM.hpp>

// Libs
#include <ArduinoJson.h>
#include <Arduino_LPS22HB.h>

DynamicJsonDocument data_json_doc(512);
DynamicJsonDocument alerts_json_doc(512);
DynamicJsonDocument limits_json_doc(1024);
SIMCOM::HttpRequest post_request;

const int ALERT_MAX_FREQUENCY = 60;

void updateAlertsList()
{
    SIMCOM::HttpResponse response = SIMCOM::getFromHTTPServer(
        String(climate_i_airtable_endpoint) +
            "alert_conditions" +
            "?fields%5B%5D=name" +
            "&fields%5B%5D=status" +
            "&fields%5B%5D=minimum_allowable" +
            "&fields%5B%5D=maximum_allowable" +
            "&fields%5B%5D=latest_alert",
        &post_request); // we can use the post request's headers for this
    if (response.success())
    {
        deserializeJson(limits_json_doc, response.body);
        response.print();
        Serial.println("Alert conditions updated.");
    }
    else
    {
        Serial.println("Unable to get alert conditions!");
        response.print();
    }
}

void setup()
{
    Serial.begin(SERIAL_BAUD);
    Wire.setPins(I2C_SDA, I2C_SCL);
    SIMCOM::init();
    post_request.addHeader("Authorization", "Bearer " + String(airtable_api_key));
    post_request.addHeader("Content-Type", "application/json");
    if (!BARO.begin())
        Serial.println("Failed to initialize pressure sensor!");

    // Populate limits doc
    updateAlertsList();
}

float compareTimestamps(String A_timestamp, String B_timestamp)
{
    if (B_timestamp.compareTo("null") == 0 || !B_timestamp)
    {
        return ALERT_MAX_FREQUENCY + 1;
    }
    return SIMCOM::formatTimeFromISO8601(A_timestamp) - SIMCOM::formatTimeFromISO8601(B_timestamp);
}

String checkForLimits(String field)
{
    JsonArray record_array = limits_json_doc["records"];
    for (JsonVariant value : record_array)
    {
        if (
            value["fields"]["name"].as<String>().compareTo(field) == 0 &&
            value["fields"]["status"].as<String>().compareTo("Active") == 0 &&
            compareTimestamps(data_json_doc["time"], value["fields"]["latest_alert"]) >= ALERT_MAX_FREQUENCY)
        {
            if (value["fields"]["minimum_allowable"].as<float>() > data_json_doc[field].as<float>() ||
                value["fields"]["maximum_allowable"].as<float>() < data_json_doc[field].as<float>())
            {
                return value["id"].as<String>();
            }
        }
    }
    return "";
}

String airtableWrap(DynamicJsonDocument doc)
{
    String tmp_string;
    serializeJson(doc, tmp_string);
    return "{\"records\": [{\"fields\":" + tmp_string + "}]}";
}

void loop()
{
    unsigned long start_time = millis();
    // Populate the datapoint
    SIMCOM::GPSResponse gps_response = SIMCOM::getGPSCoordinates();
    data_json_doc.clear();
    data_json_doc["time"] = SIMCOM::formatTimeISO8601(now());
    data_json_doc["temperature"] = BARO.readTemperature();
    data_json_doc["pressure"] = BARO.readPressure();
    data_json_doc["latitude"] = gps_response.lat;
    data_json_doc["longitude"] = gps_response.lon;
    post_request.clear(true);
    post_request.content = airtableWrap(data_json_doc);
    post_request.print();
    SIMCOM::HttpResponse response = SIMCOM::postToHTTPServer(
        String(climate_i_airtable_endpoint) + "data",
        &post_request);
    if (response.success())
    {
        Serial.println("Data point post success!");
    }
    else
    {
        Serial.println("Data point failed to post :(");
        response.print();
    }

    // Check and populate any alerts
    alerts_json_doc.clear();
    alerts_json_doc["time"] = SIMCOM::formatTimeISO8601(now());
    String temp_limit_found = checkForLimits("temperature");
    if (!temp_limit_found.isEmpty())
    {
        JsonArray alert_type_list = alerts_json_doc.createNestedArray("alert");
        alert_type_list.add(temp_limit_found);

        post_request.clear(true);
        post_request.content = airtableWrap(alerts_json_doc);
        post_request.print();
        SIMCOM::HttpResponse response = SIMCOM::postToHTTPServer(
            String(climate_i_airtable_endpoint) + "alerts",
            &post_request);

        if (response.success())
        {
            Serial.println("Alert post success!");
        }
        else
        {
            Serial.println("Alert failed to post :(");
            response.print();
        }
        updateAlertsList();
    }

    unsigned long elapsed = millis() - start_time;
    if (elapsed < 5000)
    {
        delay(5000 - elapsed);
    }
}