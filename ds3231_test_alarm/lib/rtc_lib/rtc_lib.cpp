
#include "rtc_lib.h"

extern DS3231 myRTC;
volatile uint8_t rtc_int_flag = 1;

SemaphoreHandle_t xMutex; 
/** Circular linked list schedule*/

rtc_alarm_event_t* rtc_alarm_event_create(
                            uint8_t alarm_hour,
                            uint8_t alarm_minute,
                            actuator_state actuators_state_event[3],
                            uint8_t actuator_change_state)
{
    rtc_alarm_event_t* rtc_alarm_event_new = (rtc_alarm_event_t*)malloc(sizeof(rtc_alarm_event_t));
    if(rtc_alarm_event_new == NULL)
    {
        return NULL;
    }
    rtc_alarm_event_new->alarm_hour = alarm_hour;
    rtc_alarm_event_new->alarm_minute = alarm_minute;
    rtc_alarm_event_new->actuator_change_state = actuator_change_state;
    rtc_alarm_event_new->rtc_alarm_event_next = NULL;
    /** change to config with add and remove actuator in future */
    uint8_t size = 3;


    for(uint8_t i = 0 ; i < size; i++)
    {
        rtc_alarm_event_new->actuators_state_event[i] = actuators_state_event[i];
    }
    return rtc_alarm_event_new;
}


rtc_alarm_event_t* rtc_alarm_event_insert(rtc_alarm_event_t** rtc_alarm_event_head,
                            uint8_t alarm_hour,
                            uint8_t alarm_minute,
                            //actuator_state actuators_state_event[3],
                            uint8_t actuator_change_state)
{
    //uint8_t is_it_add_to_list = 0;
    //rtc_alarm_event_t* temp = rtc_alarm_event_create(alarm_hour,alarm_minute,actuators_state_event,actuator_change_state);
     rtc_alarm_event_t* p = *rtc_alarm_event_head;
     // If the list is empty, make the new node the head
   
   /* TODO 
   first : change array of actuators -> into an extern linked list so can modify
   second : remove fixed array in parameter
   third : take extern list to know the present state of actuator from stm32
   -> actuator_state_event_head

   if the node is head node take actuator state from the head 
   other node will take from their pre node
   */
    if(p == NULL)
    {
        rtc_alarm_event_t* temp = rtc_alarm_event_create(alarm_hour,alarm_minute,actuators_state_event,actuator_change_state);
        temp-> rtc_alarm_event_next = temp;
        *rtc_alarm_event_head = temp;
    }

    // if new node in front of head node
    else if((p->alarm_hour > alarm_hour) || ((p->alarm_hour == alarm_hour) && (p->alarm_minute > alarm_minute)))
    {
        while(p->rtc_alarm_event_next != *rtc_alarm_event_head)
        {
            p = p->rtc_alarm_event_next;
        }
        p->rtc_alarm_event_next = temp;
        temp->rtc_alarm_event_next = *rtc_alarm_event_head;
        *rtc_alarm_event_head = temp;
    }
    
     // add new node
    else   
    { 
        while(p->rtc_alarm_event_next != *rtc_alarm_event_head && 
            ((p->rtc_alarm_event_next->alarm_hour < alarm_hour) || 
             ((p->rtc_alarm_event_next->alarm_hour == alarm_hour )&& (p->rtc_alarm_event_next->alarm_minute <= alarm_minute))))  
           {
               p = p->rtc_alarm_event_next; 
           }
           // if new node is isist
           if((p->alarm_hour == alarm_hour) && (p->alarm_minute == alarm_minute))
           {
               p->actuators_state_event[actuator_change_state] = actuators_state_event[actuator_change_state];
               free(temp);
           }
           else
           {
               temp->rtc_alarm_event_next = p->rtc_alarm_event_next; 
               p->rtc_alarm_event_next = temp; 
           }
            // if (p == rtc_alarm_event_head) 
            //   return temp;
        } 
    return *rtc_alarm_event_head; 
}


void rtc_alarm_event_remove(rtc_alarm_event_t** rtc_alarm_event_head,
                            uint8_t alarm_hour,
                            uint8_t alarm_minute)
{
    rtc_alarm_event_t* pre = *rtc_alarm_event_head;
    rtc_alarm_event_t* p = *rtc_alarm_event_head;
    if(p != NULL)
    {
        if((p->alarm_hour == alarm_hour )&& (p->alarm_minute == alarm_minute))
        {
            // it will not happen cause there r always 2 time start and stop but just in case :))
            if(pre->rtc_alarm_event_next == pre)
            {
                *rtc_alarm_event_head = NULL;
            }
            else
            {
                while( p->rtc_alarm_event_next != *rtc_alarm_event_head)  
                {
                    pre = pre->rtc_alarm_event_next; 
                }
                pre->rtc_alarm_event_next = p->rtc_alarm_event_next;
                *rtc_alarm_event_head = pre->rtc_alarm_event_next;
                
            }
            free(p);
        }
        else
        {
            // while( (pre->rtc_alarm_event_next != *rtc_alarm_event_head) &&
            // ((pre->rtc_alarm_event_next->alarm_hour != alarm_hour )&& (pre->rtc_alarm_event_next->alarm_minute != alarm_minute)))  
            // {
            //     pre = pre->rtc_alarm_event_next; 
            // }
            // p = pre->rtc_alarm_event_next;
            p = rtc_alarm_event_find(rtc_alarm_event_head,alarm_hour,alarm_minute);
            pre->rtc_alarm_event_next = p->rtc_alarm_event_next;
            free(p);
        }
    }
}

rtc_alarm_event_t* rtc_alarm_event_find(rtc_alarm_event_t** rtc_alarm_event_head,
                                      uint8_t alarm_hour,
                                      uint8_t alarm_minute)
{
    rtc_alarm_event_t* p = *rtc_alarm_event_head;
    rtc_alarm_event_t* rtc_alarm_event_found = NULL;

    while (p != NULL && rtc_alarm_event_found == NULL)
    {
        if (p->alarm_hour == alarm_hour && p->alarm_minute == alarm_minute)
        {
            rtc_alarm_event_found = p;
        }

        p = (p->rtc_alarm_event_next != *rtc_alarm_event_head) ? p->rtc_alarm_event_next : NULL;
    }

    return rtc_alarm_event_found;
}

rtc_alarm_event_t* rtc_alarm_event_find_pre(rtc_alarm_event_t** rtc_alarm_event_head,
                                    uint8_t alarm_hour,
                                    uint8_t alarm_minute)
{
    rtc_alarm_event_t* p = *rtc_alarm_event_head;
    //p = p->rtc_alarm_event_next;
    rtc_alarm_event_t* rtc_alarm_event_found = NULL;
    
    while (p != NULL && p->rtc_alarm_event_next != *rtc_alarm_event_head)// rtc_alarm_event_found == NULL)
    {
        if (p->rtc_alarm_event_next->alarm_hour == alarm_hour && 
            p->rtc_alarm_event_next->alarm_minute == alarm_minute)
        {
            rtc_alarm_event_found = p;
        }

        p =  p->rtc_alarm_event_next ;
    }

    return rtc_alarm_event_found;
}

void rtc_alarm_event_display(rtc_alarm_event_t* rtc_alarm_event_head)
{
    rtc_alarm_event_t* p = rtc_alarm_event_head;
    if(p == NULL) 
    {
        Serial.println("schedule is empty");
    }
    else
    {
        do { 
            Serial.println(p->alarm_hour);
            p = p->rtc_alarm_event_next; 
        } while (p != rtc_alarm_event_head);
    }
}

/*====================================================================*/
/** RTC function*/
uint8_t rtc_interpupt_callback(unsigned int gpio_pin)
{
  if(gpio_pin == CLINT )
  {
    rtc_int_flag = 1;
    return 1;
  }
  return 0;
}

void rtc_init()
{
   Wire.begin();
   xMutex = xSemaphoreCreateMutex();

   // myRTC.begin();
    myRTC.setClockMode(false);
    myRTC.setEpoch(1640995200);

    byte alarmDay;
    byte alarmHour;
    byte alarmMinute;
    byte alarmSecond;
    byte alarmBits;
    bool alarmDayIsDay;
    bool alarmH12;
    bool alarmPM;

    alarmDay = myRTC.getDate();
    alarmHour = myRTC.getHour(alarmH12, alarmPM);
    alarmMinute = myRTC.getMinute();
    alarmSecond = INT_FREQ;
    alarmBits = ALRM1_MATCH_SEC;
    alarmDayIsDay = false;

    Serial.print(alarmDay);
    Serial.print(":");
    Serial.print(alarmHour); 
    Serial.print(":");
    Serial.print(alarmMinute);
    Serial.print(":");
    Serial.print(alarmSecond);
   
    myRTC.turnOffAlarm(1);
    myRTC.setA1Time(
       alarmDay, alarmHour, alarmMinute, alarmSecond,
       alarmBits, alarmDayIsDay, alarmH12, alarmPM);
    myRTC.checkIfAlarm(1);
    myRTC.turnOnAlarm(1);

    alarmMinute = 0xFF;
    alarmBits = 0b01100000;

    myRTC.setA2Time(
        alarmDay, alarmHour, alarmMinute,
        alarmBits, alarmDayIsDay, alarmH12, alarmPM);
    myRTC.turnOffAlarm(2);
    myRTC.checkIfAlarm(2);

    xTaskCreatePinnedToCore(
        rtc_alarm_task,
        "rtc_alarm_task",
        4096,
        NULL,
        1,
        NULL,
        0
    );
}


void rtc_alarm_task(void* parameter) {
    static byte state = false;
    DateTime alarmDT;

    while (1) {
        if (rtc_int_flag) {
            xSemaphoreTake(xMutex, portMAX_DELAY);

            alarmDT = RTClib::now();

            myRTC.turnOffAlarm(1);
            myRTC.checkIfAlarm(1);
            rtc_int_flag = 0;
            state = ~state;
            //digitalWrite(LED, state);

            Serial.print("Turning LED ");
            Serial.print((state ? "ON" : "OFF"));
            Serial.print(" at ");
            Serial.print(alarmDT.hour());
            Serial.print(":");
            Serial.print(alarmDT.minute());
            Serial.print(":");
            Serial.println(alarmDT.second());

            uint32_t nextAlarm = alarmDT.unixtime();
            nextAlarm += INT_FREQ;
            alarmDT = DateTime(nextAlarm);

            myRTC.setA1Time(
                alarmDT.day(), alarmDT.hour(), alarmDT.minute(), alarmDT.second(),
                ALRM1_MATCH_SEC, false, false, false
            );

            myRTC.turnOnAlarm(1);
            myRTC.checkIfAlarm(1);

            xSemaphoreGive(xMutex);
            
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);  // Adjust delay as needed
    }
}


/** RTC ana CLL schedule function */


void rtc_add_alarm_event_schedule(rtc_alarm_event_t** rtc_alarm_event_head,
                         uint8_t alarm_hour_start,
                         uint8_t alarm_minute_start,
                         uint8_t alarm_hour_end,
                         uint8_t alarm_minute_end,
                         uint8_t actuator_change_state)
{
        /*TODO :
            checjk time vs 
            nếu chưa có thì tạo
            có rồi thì thêm function
            -> refix code 
            -> check tới bé hơn cái time nếu chưa có thfi tạo alarm adđ vô nếu có thì thêm

        */

}

void rtc_remove_alarm_event_schedule(rtc_alarm_event_t** rtc_alarm_event_head,
                            uint8_t alarm_hour_start,
                            uint8_t alarm_minute_start,
                            uint8_t alarm_hour_end,
                            uint8_t alarm_minute_end,
                            uint8_t actuator_change_state);

void rtc_start_alarm_event_schedule(rtc_alarm_event_t** rtc_alarm_event_head);