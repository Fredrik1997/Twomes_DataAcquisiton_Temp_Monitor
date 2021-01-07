#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

#define testPayload "{'id':'FF:FF:FF:FF:FF','dataSpec':{'lastTime':1606914671,'interval':10,'total':6},'data':{'pipeTemp1':[50.1,51.2,52.3,53.4,54.5],'pipeTemp2':[50.1,51.2,52.3,53.4,54.5]}}"
#define length_testPayload 169

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //REVIEW, this is a broadcast address

typedef struct struct_message
{
  char sendMessage[length_testPayload + 1];
} struct_message;

struct_message myData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

#define timeMeasurePin 14

char testMessage[] = testPayload;

void setup()
{
  pinMode(timeMeasurePin, OUTPUT);

  Serial.begin(115200);
  Serial.println("Device has started");

  Serial.print("Testmessage: ");
  Serial.println(testMessage);
  Serial.print("has lenght: ");
  Serial.println(strlen(testMessage));

  strcpy(myData.sendMessage, testMessage);
  Serial.print("myData.sendMessage: ");
  Serial.println(myData.sendMessage);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 6;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  // Set values to send

  digitalWrite(timeMeasurePin, HIGH); //for time logging with scope
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  digitalWrite(timeMeasurePin, LOW); //for time logging with scope

  if (result == ESP_OK)
  {
    // Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
  delay(10000);
}