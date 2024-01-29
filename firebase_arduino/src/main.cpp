
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

#include <Arduino.h>

#include <WiFi.h>

#include <FirebaseESP32.h>
#include <stdio.h>
#include <String>
#include <stdint.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


/* 1. Define the WiFi credentials */


#define WIFI_SSID "MI 8 Pro"
#define WIFI_PASSWORD "123456789"

// #define WIFI_SSID "HíHíHí"
// #define WIFI_PASSWORD "hihihi858585"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

// /* 2. Define the API Key */
// #define API_KEY "AIzaSyAHzD02XGJEV8anl7CXphWyJhNotsoIxm8"

// /* 3. Define the RTDB URL */
// #define DATABASE_URL "https://myproject-b214a-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app


/* 2. Define the API Key */
#define API_KEY "AIzaSyDgReLUPXPzDwMjOO5U5Y66RuTCFwUBJR0"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://aquasys-e55d4-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "phuchocnhung@gmail.com"
#define USER_PASSWORD "phucphuc"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseData stream;

FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

unsigned long sendDataPrevMillis = 0;

 long count = 0;

String parentPath = "/Actuators";
String childPath[2] = {"/Actuators_water","/Actuators_environment"};

SemaphoreHandle_t semaphore_firebase_is_free;

void read_fb(void *pvParameters);
void send_fb(void *pvParameters);

volatile bool dataChanged = false;


void streamCallback(MultiPathStreamData stream)
{
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.printf("path: %s, event: %s, type: %s, value: %s%s", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");
    }
  }

  Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since the stream connected
  // and read once and will not update until stream reconnection takes place.
  // This max value will be zero as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup()
{

  Serial.begin(115200);
  semaphore_firebase_is_free = xSemaphoreCreateBinary();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);

  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.


  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  // You can use TCP KeepAlive For more reliable stream operation and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP8266#enable-tcp-keepalive-for-reliable-http-streaming
  // You can use keepAlive in ESP8266 core version newer than v3.1.2.
  // Or you can use git version (v3.1.2) https://github.com/esp8266/Arduino
  // stream.keepAlive(5, 5, 1);
if (!Firebase.beginMultiPathStream(stream, parentPath))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setMultiPathStreamCallback(stream, streamCallback, streamTimeoutCallback);

  /** Timeout options, below is default config.

  //Network reconnect timeout (interval) in ms (10 sec - 5 min) when network or WiFi disconnected.
  config.timeout.networkReconnect = 10 * 1000;

  //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
  config.timeout.socketConnection = 30 * 1000;

  //ESP32 SSL handshake in ms (1 sec - 2 min). This option doesn't allow in ESP8266 core library.
  config.timeout.sslHandshake = 2 * 60 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  */
  xTaskCreatePinnedToCore(send_fb, "send", 8192, NULL, configMAX_PRIORITIES, NULL,1);

  xSemaphoreGive(semaphore_firebase_is_free);
 
  
}
void send_fb(void *pvParameters)
{
  while(1)
  {
    // if (Firebase.ready() && (millis() - sendDataPrevMillis > 10000 || sendDataPrevMillis == 0))
    // {
      if( xSemaphoreTake(semaphore_firebase_is_free, 10))
      {
        sendDataPrevMillis = millis();
        Serial.println( millis());
        Serial.println("send");

      // Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/Actuators_environment/0/status"), count) ? "ok" : fbdo.errorReason().c_str());
        json.set("Humi/", F("5"));
        json.set("Light/", F("5"));
        json.set("Moisture/", F("5"));
        json.set("Temp/", F("5"));
        json.set("Water_level/", F("5"));
        json.set("pH/", F("5"));
        Serial.printf("Set json... %s\n", Firebase.set(fbdo, F("/val_sensor"), json) ? "ok" : fbdo.errorReason().c_str());
        Serial.println( millis());
        xSemaphoreGive(semaphore_firebase_is_free);
        vTaskDelay(10000);
      }
    //}
  
  }
  vTaskDelete(NULL);
}
void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.


  

  if (dataChanged)
  {
    dataChanged = false;
    // When stream data is available, do anything here...
  }

  // After calling stream.keepAlive, now we can track the server connecting status
  if (!stream.httpConnected())
  {
    // Server was disconnected!
  }
}