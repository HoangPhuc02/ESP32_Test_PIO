#ifndef __RTC_LIB_H
#define __RTC_LIB_H
/** UART config*/
/*Pin */
#include <Arduino.h>

#include <stdio.h>
#include <String>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <freertos/semphr.h>
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <stdlib.h>


#include <Wire.h>
#include <DS3231.h>

#include <actuator_lib.h>


#define INT_FREQ 5UL

#define CLINT 4
#define ACT1 16
#define ACT2 17
#define ACT3 5

enum actuator_state
{
  actuator_off = 0u,
  actuator_on,
  actuator_not_change
};
typedef struct rtc_event
{
  /* data */
  //rtc_event* pre_rtc_alarm_event_t;
  rtc_event* rtc_alarm_event_next;
  uint8_t alarm_hour;
  uint8_t alarm_minute;
  actuator_state actuators_state_event[3];
  uint8_t is_trigger;
  uint8_t actuator_change_state;
}rtc_alarm_event_t;

#define ALRM1_MATCH_EVERY_SEC  0b1111
#define ALRM1_MATCH_SEC        0b1110
#define ALRM1_MATCH_MIN_SEC    0b1100
#define ALRM1_MATCH_HR_MIN_SEC 0b1000

#define ALRM2_ONCE_PER_MIN     0b111
#define ALRM2_MATCH_MIN        0b110
#define ALRM2_MATCH_HR_MIN     0b100

/** Circular linked list schedule*/
/**
 * @brief create a node in circular liked list of event
 * 
 * event_head -> event -> event  <--- create a node 
 *  ^                       l
 *  l-----------------------<
 * @param rtc_alarm_event_head  head of circular linked list event
 * @param alarm_hour            hour event start
 * @param alarm_minute          minute event start
 * @param actuators_state_event state of all actuators when start event
 * @param actuator_change_state state of the actuator change in this event
 * @return rtc_alarm_event* 
 */

rtc_alarm_event_t* rtc_alarm_event_create(
                            uint8_t alarm_hour,
                            uint8_t alarm_minute,
                            actuator_state actuators_state_event[3],
                            uint8_t actuator_change_state);

/**
 * @brief insert a node into circular liked list of event 
 * require rtc_alarm_event* rtc_alarm_event_create(---);
 * 
 * event_head -> event -> event  <--- insert a node 
 *  ^                       l
 *  l-----------------------<
 * @param rtc_alarm_event_head  head of circular linked list event
 * @param alarm_hour            hour event start
 * @param alarm_minute          minute event start
 * @param actuators_state_event state of all actuators when start event
 * @param actuator_change_state state of the actuator change in this event
 * @return rtc_alarm_event* 
 */

rtc_alarm_event_t* rtc_alarm_event_insert(rtc_alarm_event_t** rtc_alarm_event_head,
                            uint8_t alarm_hour,
                            uint8_t alarm_minute,
                            actuator_state actuators_state_event[3],
                            uint8_t actuator_change_state);

/**
 * @brief remove a node out circular liked list of event
 *
 * event_head -> event  x  event  <--- remove a node
 *  ^              l
 *  l--------------<
 *
 * remember to free node
 * used in
 *
 * @param rtc_alarm_event_t_head  head of circular linked list event
 * @param alarm_hour            hour of event need to remove
 * @param alarm_minute          minute of event need to remove
 */
void rtc_alarm_event_remove(rtc_alarm_event_t** rtc_alarm_event_head,
                            uint8_t alarm_hour,
                            uint8_t alarm_minute);

rtc_alarm_event_t* rtc_alarm_event_find(rtc_alarm_event_t** rtc_alarm_event_head,
                                    uint8_t alarm_hour,
                                    uint8_t alarm_minute);

rtc_alarm_event_t* rtc_alarm_event_find_pre(rtc_alarm_event_t** rtc_alarm_event_head,
                                    uint8_t alarm_hour,
                                    uint8_t alarm_minute);

void rtc_alarm_event_display(rtc_alarm_event_t* rtc_alarm_event_head);





/*====================================================================*/
// Mutex for protecting shared resources


/** RTC function*/
uint8_t rtc_interpupt_callback(unsigned int gpio_pin);
void rtc_alarm_task(void* parameter);
void rtc_init(void);

/** RTC ana CLL schedule function */


void rtc_add_alarm_event_schedule(rtc_alarm_event_t** rtc_alarm_event_head,
                                  uint8_t alarm_hour_start,
                                  uint8_t alarm_minute_start,
                                  uint8_t alarm_hour_end,
                                  uint8_t alarm_minute_end,
                                  uint8_t actuator_change_state);

void rtc_remove_alarm_event_schedule(rtc_alarm_event_t** rtc_alarm_event_head,
                                    uint8_t alarm_hour_start,
                                    uint8_t alarm_minute_start,
                                    uint8_t alarm_hour_end,
                                    uint8_t alarm_minute_end,
                                    uint8_t actuator_change_state);

void rtc_start_alarm_event_schedule(rtc_alarm_event_t** rtc_alarm_event_head);

#endif