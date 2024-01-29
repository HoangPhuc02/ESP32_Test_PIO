#ifndef __SENSOR_LIB_H
#define __SENSOR_LIB_H
#include <Arduino.h>

#include <stdio.h>
#include <string.h>
#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <stdlib.h>


/*======================= SENSOR Declare ========================*/
//#define MAX_SENSOR_DATA 6

typedef enum {
  Temp,
  Humi,
  ADC1,
  ADC2,
  ADC3,
  ADC4,
  MAX_SENSOR_DATA   
} sensor_data_type_t;
// String sensor_data[MAX_SENSOR_DATA] = {};


uint8_t sensor_check(String sensor_data[], int size);

/*================================================================*/

#endif