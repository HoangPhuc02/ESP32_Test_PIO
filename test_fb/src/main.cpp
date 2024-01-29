#include <Arduino.h>
#include <stdio.h>
#include <String>
#include <stdint.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <WiFi.h>
#include <FirebaseESP32.h>
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/* 1. Define the WiFi credentials */


#define WIFI_SSID "MI 8 Pro"
#define WIFI_PASSWORD "123456789"

// #define WIFI_SSID "HíHíHí"
// #define WIFI_PASSWORD "hihihi858585"

// #define WIFI_SSID "Symbol Coffee Tea"
// #define WIFI_PASSWORD "Symbolcoffee"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDgReLUPXPzDwMjOO5U5Y66RuTCFwUBJR0"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://aquasys-e55d4-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "phuchocnhung@gmail.com"
#define USER_PASSWORD "phucphuc"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;


typedef enum
{
    Relay = 0,
    Servo,
    Led,
    MAX_DEVICE
} Actuator_Type_t;
typedef enum
{
    ACTUATOR_OFF = 0,
    ACTUATOR_ON
} Actuator_State_t;
typedef struct
{
    Actuator_Type_t ActuatorId;
    Actuator_State_t ActuatorState;
    /* data */
} Actuator_Init_t;

Actuator_Init_t actuatorData;

#define MAX_SENSOR_DATA 6
String sensorData[MAX_SENSOR_DATA] = {"10.6", "20.8", "1093", "2048", "100", "4096"};


SemaphoreHandle_t dataSemaphore;

void updateFirebaseData(String nodePath, FirebaseJson* json);

void sensorDataTask(void *pvParameters)
{
    while (1)
    {
      uint32_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("LoopTask stack high water mark: %u words\n", stackHighWaterMark);
        for (int i = 0; i < MAX_SENSOR_DATA; ++i)
        {
            sensorData[i] = "SensorValue" + String(i + 1);
        }

        xSemaphoreGive(dataSemaphore);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void actuatorDataTask(void *pvParameters)
{
    while (1)
    {
      uint32_t stackHighWaterMark2 = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("LoopTask stack high water mark: %u words\n", stackHighWaterMark2);
        actuatorData.ActuatorId = Relay;
        actuatorData.ActuatorState = ACTUATOR_ON;

        xSemaphoreGive(dataSemaphore);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}
void loop_Task(void *pvParameters)
{
    while(1)
    {
   if (Firebase.ready() && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        FirebaseJson json;
        String path = "/Sensors_environment" ;
        String upPath;
        if (xSemaphoreTake(dataSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // Update sensor data paths
            for (int i = 0; i < 3; ++i)
            {
                upPath = path + "/" + String(i) + "/value";
                Serial.printf("Set sensor... %s\n", Firebase.setString(fbdo, upPath.c_str(),sensorData[i]) ? "ok" : fbdo.errorReason().c_str());
                //json.set("sensorData/sensor" + String(i + 1), sensorData[i]);
            }
            // Update actuator data paths
            //Firebase.setInt(fbdo,"actuatorData/ActuatorId", static_cast<int>(actuatorData.ActuatorId));
            //Firebase.setInt(fbdo,"actuatorData/ActuatorState", static_cast<int>(actuatorData.ActuatorState));
            //databasePath = "test";
            // Send the entire JSON data to Firebase
            
            // Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
            //Serial.printf("Set json... %s\n", Firebase.set(fbdo, path.c_str(),json) ? "ok" : fbdo.errorReason().c_str());
        }
        
        Serial.println();
        Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/Actuators_environment/0/status")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    }
    }
    vTaskDelete(NULL);
}

// void updateFirebaseData(String nodePath, FirebaseJson* json)
// {
//     String path = "/" + nodePath ;

//     // Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

//     Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, path.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    
//     // if (Firebase.RTDB.setJSON(&firebaseData, path.c_str(), &json))
//     // {
//     //     Serial.println(nodePath + " data updated successfully");
//     // }
//     // else
//     // {
//     //     Serial.println("Failed to update " + nodePath + " data");
//     //     Serial.println("Reason: " + firebaseData.errorReason());
//     // }
// }
void setup()
{
    Serial.begin(115200);

  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  /* Remove WiFi event
  Serial.print("WiFi Event ID: ");
  Serial.println(eventID);
  WiFi.removeEvent(eventID);*/


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

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP8266#about-firebasedata-object
  // fbdo.keepAlive(5, 5, 1);

  /** Timeout options.

  //Network reconnect timeout (interval) in ms (10 sec - 5 min) when network or WiFi disconnected.
  config.timeout.networkReconnect = 10 * 1000;

  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  Note:
  The function that starting the new TCP session i.e. first time server connection or previous session was closed, the function won't exit until the
  time of config.timeout.socketConnection.

  You can also set the TCP data sending retry with
  config.tcp_data_sending_retry = 1;

  */
    // Create a semaphore to synchronize data updates
    dataSemaphore = xSemaphoreCreateBinary();

    // Create tasks
    xTaskCreatePinnedToCore(sensorDataTask, "SensorDataTask", 8192, NULL, 1, NULL,0);
    xTaskCreatePinnedToCore(actuatorDataTask, "ActuatorDataTask", 8192, NULL, 1, NULL,0);
    xTaskCreatePinnedToCore(loop_Task, "LoopTask", 32768, NULL, 2, NULL,1);

   // Start the FreeRTOS scheduler
    vTaskStartScheduler();
}

void loop()
{
    // This loop will not be executed as FreeRTOS scheduler is running
    // uint32_t stackHighWaterMark3 = uxTaskGetStackHighWaterMark(NULL);
    // Serial.printf("LoopTask stack high water mark: %u words\n", stackHighWaterMark3);
    // if (xSemaphoreTake(dataSemaphore, portMAX_DELAY) == pdTRUE)
    // {
    //     // Update sensor data paths
    //     for (int i = 0; i < MAX_SENSOR_DATA; ++i)
    //     {
    //         json.set("sensorData/sensor" + String(i + 1), sensorData[i]);
    //     }
    //     // Update actuator data paths
    //     json.set("actuatorData/ActuatorId", static_cast<int>(actuatorData.ActuatorId));
    //     json.set("actuatorData/ActuatorState", static_cast<int>(actuatorData.ActuatorState));
    //     databasePath = "test";
    //     // Send the entire JSON data to Firebase
    //     String path = "/" + databasePath  ;
    //     // Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    //     Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, path.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    // }
    // if (Firebase.ready() && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0))
    // {
    //     sendDataPrevMillis = millis();
    //     FirebaseJson json;
    //     String path = "/Sensors_environment" ;
    //     String upPath;
    //     if (xSemaphoreTake(dataSemaphore, portMAX_DELAY) == pdTRUE)
    //     {
    //         // Update sensor data paths
    //         for (int i = 0; i < 3; ++i)
    //         {
    //             upPath = path + "/" + String(i) + "/value";
    //             Serial.printf("Set sensor... %s\n", Firebase.setString(fbdo, upPath.c_str(),sensorData[i]) ? "ok" : fbdo.errorReason().c_str());
    //             //json.set("sensorData/sensor" + String(i + 1), sensorData[i]);
    //         }
    //         // Update actuator data paths
    //         //Firebase.setInt(fbdo,"actuatorData/ActuatorId", static_cast<int>(actuatorData.ActuatorId));
    //         //Firebase.setInt(fbdo,"actuatorData/ActuatorState", static_cast<int>(actuatorData.ActuatorState));
    //         //databasePath = "test";
    //         // Send the entire JSON data to Firebase
            
    //         // Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    //         //Serial.printf("Set json... %s\n", Firebase.set(fbdo, path.c_str(),json) ? "ok" : fbdo.errorReason().c_str());
    //     }
        
    //     Serial.println();
    //     Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/Actuators_environment/0/status")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    //}
    //Firebase.getString(fbdo,"/test/actuatorData/ActuatorId");
    
    
    
}

// void updateFirebaseData(const String &sensorNodePath, const String sensorValues[MAX_SENSOR_DATA],
//                         const String &actuatorNodePath, const Actuator_Init_t &actuatorData)
// {
//     // Create a JSON object with sensor data
//     FirebaseJson sensorJson;
//     for (int i = 0; i < MAX_SENSOR_DATA; ++i)
//     {
//         sensorJson.set("sensor" + String(i + 1), sensorValues[i]);
//     }

//     // Update sensor data in Firebase
//     if (Firebase.setJSON(firebaseData, sensorNodePath, sensorJson))
//     {
//         Serial.println("Sensor data updated successfully");
//     }
//     else
//     {
//         Serial.println("Failed to update sensor data");
//         Serial.println("Reason: " + firebaseData.errorReason());
//     }

//     // Create a JSON object with actuator data
//     FirebaseJson actuatorJson;
//     actuatorJson.set("ActuatorId", static_cast<int>(actuatorData.ActuatorId));
//     actuatorJson.set("ActuatorState", static_cast<int>(actuatorData.ActuatorState));

//     // Update actuator data in Firebase
//     if (Firebase.setJSON(firebaseData, actuatorNodePath, actuatorJson))
//     {
//         Serial.println("Actuator data updated successfully");
//     }
//     else
//     {
//         Serial.println("Failed to update actuator data");
//         Serial.println("Reason: " + firebaseData.errorReason());
//     }
// }