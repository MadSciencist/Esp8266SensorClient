/* settings:
 board: ESP-12
 flash: 4MB (1MB SPIFFS)
 F_CPU: 80 MHz
 Upload baud: 115200
 Debug_port: Serial
 Debug_levle: All
*/
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2 // DS18B20 pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ESP8266WiFiMulti AccessPoints;

String myCredentials = "{\"username\": \"homeAutomationSensor\", \"password\": \"homeAutomationSensorPassword\"}";
String cloudUrl = "http://192.168.0.223/api/";
String identifier = "dev";

const size_t bufferSize = JSON_OBJECT_SIZE(1) + 220;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  /* add your access points here, you can use more than 2 or just 1 */
  AccessPoints.addAP("ssid1", "password1");
  AccessPoints.addAP("ssid2", "password2");
}

void loop()
{
  if ((AccessPoints.run() == WL_CONNECTED))
  {
    HTTPClient getTokenHttpSession;
    getTokenHttpSession.begin(cloudUrl + "token");
    getTokenHttpSession.addHeader("Content-Type", "application/json");
    getTokenHttpSession.addHeader("Content-Length", (String)(myCredentials.length()));

    if (getTokenHttpSession.POST(myCredentials) == HTTP_CODE_OK)
    {
      /* the request is sucessfull, parse the JSON response and post data with received token */
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject &response = jsonBuffer.parseObject(getTokenHttpSession.getString());
      const char *token = response["token"];
      String headerAuthorizationValue = "Bearer ";
      headerAuthorizationValue += token;
      Serial.println(headerAuthorizationValue);

      HTTPClient postDataHttpSession;
      postDataHttpSession.begin(cloudUrl + "sensors/specified");
      postDataHttpSession.addHeader("Content-Type", "application/json");
      postDataHttpSession.addHeader("Authorization", headerAuthorizationValue);

      String payload = "{\"identifier\": \"" + identifier + "\", \"data\": \"" + GetSensorDataString() + "\"}";
      Serial.println(payload);

      postDataHttpSession.POST(payload);
      postDataHttpSession.end();
    }
    getTokenHttpSession.end();
  }

  delay(5000);
  //ESP.deepSleep(20e6);
}

String GetSensorDataString()
{
  DS18B20.requestTemperatures();
  float temperature = DS18B20.getTempCByIndex(0);
  float voltage = analogRead(A0);
  String data = "temperature: " + (String)temperature + " * " + "sun: " + (String)voltage;
  Serial.println(data);
  return data;
}

