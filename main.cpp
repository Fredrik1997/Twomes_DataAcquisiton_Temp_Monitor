#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <esp_wifi.h>
#include <esp_bt_main.h>

#include <esp_now.h>
#include <WiFi.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  4        /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_CONVERSION 1     /* Time ESP32 will go to sleep for conversion(in seconds) */
#define INTERVAL_US     5000000   /* desired interval between conversions */
#define samples 5

RTC_DATA_ATTR int samples_in_mem = 0;
RTC_DATA_ATTR bool wake_after_conversion = 0;
RTC_DATA_ATTR int64_t previous_time = 0;
RTC_DATA_ATTR int64_t time_correction = 400000; //initial time correction

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //REVIEW, this is a broadcast address

typedef struct struct_message
{
  float temperature1;
  float temperature2;
} struct_message;

struct_message myData;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 25;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup(){
  setCpuFrequencyMhz(80); // to set the cpu frequency to 80Mhz to be more energy efficient
  
  Serial.begin(115200);
  //delay(1000); //Take some time to open up the Serial Monitor(testing only)
  
  //esp_wifi_stop(); //disable wifi
  //esp_bluedroid_disable(); //disable bluetooth

  if (wake_after_conversion == 0){
  sensors.begin(); //initialize sensor
  sensors.setWaitForConversion(false); //disable the wait for conversion
  sensors.requestTemperatures(); //start conversion
  
  esp_sleep_enable_timer_wakeup(TIME_TO_CONVERSION * uS_TO_S_FACTOR); //sleep for 1 second during conversion
  
  //Serial.println(wake_after_conversion);
  //Serial.println("Going to sleep now");
  //Serial.flush();
  wake_after_conversion = 1; //set conversion is started bit
  
  esp_deep_sleep_start(); //go to deepsleep 
  }
  
  else{ 
  sensors.begin(); //initialize again to find the device adress of the sensor
  myData.temperature1 = sensors.getTempCByIndex(0);  // get the temperature of the sensor 
  Serial.println(" "); // to get a readable temperature
      // if the sensor isn't connected the temperature wil read -127. Because this value isn't in the supported range of the sensor, this value doesn't need to be send
    if (myData.temperature1 == -127){
      Serial.println("Sensor niet verbonden");
    }
    else{ 
      Serial.print(myData.temperature1);
      Serial.println("ºC");
    }
  
  struct timeval tv;
  gettimeofday(&tv,NULL);
  int64_t time_us = (int64_t)tv.tv_sec * 1000000L + (int64_t)tv.tv_usec;  //code needed to get the time in microseconds
  int32_t time_delta = time_us - previous_time; //calculate the time that is elapsed since the last measurement
  previous_time = time_us; // write current time to memory for next cycle
  Serial.println(time_delta);
  time_correction+=(time_delta-INTERVAL_US);  // finetune time correction
  //Serial.println((int32_t)time_correction);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR - time_correction); //sleep until next temperature reading 
  //Serial.println(wake_after_conversion);
  //Serial.println("Going to sleep now");
  //Serial.flush();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

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
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result != ESP_OK)
  {
     Serial.println("Error sending the data");
  }
  wake_after_conversion = 0; //reset conversion is started bit
  
  esp_deep_sleep_start(); //go to deepsleep
  }
}

void loop(){
  //This is not going to be called
}