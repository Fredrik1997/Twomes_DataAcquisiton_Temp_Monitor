#include <WiFi.h>
#include <Arduino.h>
#include <identification.h>
#include <HTTPClient.h>

#define testPayload "{\"id\":\"FF:FF:FF:FF:FF\",\"dataSpec\":{\"lastTime\":1606914671,\"interval\":10,\"total\":6},\"data\":{\"pipeTemp1\":[50.1,51.2,52.3,53.4,54.5],\"pipeTemp2\":[50.1,51.2,52.3,53.4,54.5]}}"
#define length_testPayload 169

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //REVIEW, this is a broadcast address

#define timeMeasurePin 14

char testMessage[] = testPayload;

const char *ssid = userWiFiSSID;         //ssid internet access point, configure in utils.h
const char *password = userWiFipassword; //password internet access point, configure in utils.h

const char *serverName = userServerName;

uint16_t serverPort = userServerPort;

void setup()
{
  pinMode(timeMeasurePin, OUTPUT);

  Serial.begin(115200);
  Serial.println("Device has started");

  Serial.print("Testmessage: ");
  Serial.println(testMessage);
  Serial.print("has lenght: ");
  Serial.println(strlen(testMessage));

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, password);
  }

  Serial.println("Connected");
}

boolean sendMessage()
{
  // Set values to send
  HTTPClient http;
  String httpRequestData = testPayload;

  digitalWrite(timeMeasurePin, HIGH); //for time logging with scope
  if (!http.begin(serverName, serverPort, "/set/house/centralHeatingTemperature"))
  {
    return false;
  }
  //http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(httpRequestData);
  // Serial.print("httpRequestData : ");
  // Serial.println(httpRequestData);

  // Serial.print("HTTP Response code: ");
  // Serial.println(httpResponseCode);

  http.end();
  digitalWrite(timeMeasurePin, LOW); //for time logging with scope

  if (httpResponseCode == 200)
  {
    Serial.println("send succeed");
    return true;
  }
  return false;
}

void loop()
{

  sendMessage();

  delay(5000);
}