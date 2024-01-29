/* WiFi scan Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#include <Arduino.h>
#include "esp_log.h"
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include <FirebaseESP32.h>
#include "lcdMenu.h"
//#include <string.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
/*================== LCD ====================*/
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Menu* menu = &MainMenu;
static const unsigned char logo8_cursor[] =
{
  B00000000, 
  B11000000, 
  B11110000, 
  B11111100, 
  B11111111, 
  B11111100, 
  B11110000, 
  B11000000,  
};
/*===============Fire base ==================*/


// Insert your network credentials
#define WIFI_SSID "Symbol Coffee Tea"
#define WIFI_PASSWORD "Symbolcoffee"

// #define WIFI_SSID "HíHíHí"
// #define WIFI_PASSWORD "hihihi858585"

// #define WIFI_SSID "MI 8 Pro"
// #define WIFI_PASSWORD "123456789"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAHzD02XGJEV8anl7CXphWyJhNotsoIxm8"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "phuchocnhung@gmail.com"
#define USER_PASSWORD "phucphuc"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://myproject-b214a-default-rtdb.firebaseio.com"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
// Parent Node (to be updated in every loop)
String parentPath;

String data;
int timestamp;
FirebaseJson json;


void wifiScan() ;
void MenuInit();
void MenuDisplay(Menu *menu, uint8_t select);

void Wifi_Init();

void firebase_init(void);
void firebase_update_data_task(void *arg);








#if !CONFIG_AUTOSTART_ARDUINO

void arduinoTask(void *pvParameter) {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    delay(100);

    while(1) {
        //Serial.println("call from esp-idf");
        wifiScan();
        // Wait a bit before scanning again
        vTaskDelay(5000);
    }
}

extern "C" void app_main()
{
    // initialize arduino library before we start the tasks
    initArduino();
     Serial.begin(115200);
    //  MenuInit();
    //  MenuDisplay(menu,menu->cur_cursor);
    // xTaskCreatePinnedToCore(&arduinoTask, "arduino_task", 4096, NULL, 5, NULL,1);

    Wifi_Init();
    firebase_init();

}

#else
void setup() {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.begin(115200);
    delay(100);
}

void loop() {
   // Serial.println("call from arduino");
    wifiScan();
    // Wait a bit before scanning again
    delay(5000);
}
#endif 



void wifiScan() {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        //ESP_LOGI("wifi","no networks found");
       Serial.println("no networks found");
    } else {
        // ESP_LOGI("wifi","%d",n);
        // ESP_LOGI("wifi","networks found");
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            //ESP_LOGI("wifi","%d : %s (%d)",i+1,&WiFi.SSID(i)[0],WiFi.RSSI(i));
            vTaskDelay(10);
        }
    }
    Serial.println("");
}
void MenuInit(void)
{
    display.begin(i2c_Address, true); // Address 0x3C default
    display.display();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    MenuDisplay(&MainMenu,MainMenu.cur_cursor);
}

void MenuDisplay(Menu *menu, uint8_t select)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(menu->Title);
    for(int i = 0; i < menu->maxSelect;i++)
    {
        display.setCursor(0, 12*(i+1));
        display.println(menu->MenuList[i]);
    }   
    display.drawBitmap(0, 12*select,  logo8_cursor, 8, 8, 1);
    
    display.display();
    /*
    display.drawBitmap(0, 12,  logo8_cursor, 8, 8, 1);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    display.drawBitmap(0, 12,  logo8_cursor, 8, 8, 0);
    display.drawBitmap(0, 24,  logo8_cursor, 8, 8, 1);
    display.display();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    display.drawBitmap(0, 24,  logo8_cursor, 8, 8, 0);
    display.drawBitmap(0, 36,  logo8_cursor, 8, 8, 1);
    display.display();
    */
}

void Wifi_Init()
{
   
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print( "Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.print("You're now fucking connect to wifi");
    delay(1000);
    // ESP_LOGI("WIFI",(string)WiFi.localIP());
}

/*====================== Firebase Function =======================*/
void firebase_init(void)
{
      // Assign the api key (required)
    config.api_key = API_KEY;

    // Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // Assign the RTDB URL (required)
    config.database_url = DATABASE_URL;



    // Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    Serial.println ("token success");
   

  //  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    // Assign the maximum retry of token generation
    //config.max_token_generation_retry = 100;
        
    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(4096);
    Serial.println ("reponse success");
    Firebase.begin(&config, &auth);
    Serial.println ("connect success");
     // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectWiFi(true);

    Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
    // Getting the user UID might take a few seconds
    // Serial.println("Getting User UID");
    // while ((auth.token.uid) == "") {
    //     Serial.print('.');
    //     delay(1000);
    // }
    // // Print user UID
    // uid = auth.token.uid.c_str();
    // Serial.print("User UID: ");
    // Serial.println(uid);

    // Update database path
    //databasePath = String("/UsersData/" + uid + "/readings");

    //firebase_actuator_queue =  xQueueCreate(10, sizeof(Actuator_Init_t));

   // xTaskCreatePinnedToCore(firebase_update_data_task, "firebase_update_data_task", 8192, NULL, configMAX_PRIORITIES - 2, NULL, 0);
    // xQueueSend(firebase_actuator_queue, &actuators[0], portMAX_DELAY);
    // xQueueSend(firebase_actuator_queue, &actuators[1], portMAX_DELAY);
    // xQueueSend(firebase_actuator_queue, &actuators[2], portMAX_DELAY);
}

void firebase_update_data_task(void *pvParameters)
{
    // static const char *FB_UP_TASK_TAG = "FB_UP_TASK";
    // Actuator_Init_t actuator_firebase_update_p;
    // esp_log_level_set(FB_UP_TASK_TAG , ESP_LOG_INFO);
   // int i = 0;
    unsigned long count = 0;
    for (;;)
    {

        if (Firebase.ready())
        {
            if (count == 0)
            {
                json.set("value/round/" + String(count), F("cool!"));
                json.set(F("value/ts/.sv"), F("timestamp"));
                Firebase.RTDB.set(&fbdo, F("/test/json"), &json);
            }
            else
            {
                json.add(String(count), F("smart!"));
                Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/test/json/value/round"), &json) ? "ok" : fbdo.errorReason().c_str());
            }

            Serial.println();

        count++;
            //Serial.printf("Set json... %s\n",  ? "ok" : fbdo.errorReason().c_str());
            
        }
        vTaskDelay(15000 / portTICK_PERIOD_MS);
        
        // if (xQueueReceive( firebase_actuator_queue, &actuator_firebase_update_p, portMAX_DELAY))
        // {
        //     parentPath= databasePath + "/" + "actuator";
           
        //     json.set(actuator_name[actuator_firebase_update_p.ActuatorId], actuator_state[actuator_firebase_update_p.ActuatorState]);
        //     Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json);
        //     //Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

        //     // vTaskDelay(2000 / portTICK_PERIOD_MS);
        // }
       
    }
}
/*================================================================*/