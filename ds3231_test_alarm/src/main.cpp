#include <Wire.h>
#include <DS3231.h>
#include <rtc_lib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>

DS3231 myRTC;

#define GPIO_INPUT_PIN_SEL 1ULL << CLINT
#define LED 16
#define ESP_INTR_FLAG_DEFAULT 0




void configure_input(void);
void IRAM_ATTR gpio_isr_handler(void *parameter);

rtc_alarm_event* rtc_alarm_event_head = NULL;
actuator_state actuators_state_event[3] = {actuator_on,actuator_off,actuator_off};


void test_algorithm_1_time()
{
    delay(1000);
    //38us
    digitalWrite(LED ,HIGH);  
    rtc_alarm_event_head = rtc_alarm_event_insert(&rtc_alarm_event_head,2,10,actuators_state_event,0);
    digitalWrite(LED ,LOW); 
    actuators_state_event[2] = actuator_on;
    delay(1000);
    //31us
    digitalWrite(LED ,HIGH); 
    rtc_alarm_event_head = rtc_alarm_event_insert(&rtc_alarm_event_head,3,10,actuators_state_event,1);
    digitalWrite(LED ,LOW); 
    delay(1000);
    //11us
    digitalWrite(LED ,HIGH); 
    rtc_alarm_event_head = rtc_alarm_event_insert(&rtc_alarm_event_head,1,10,actuators_state_event,1);
    digitalWrite(LED ,LOW); 
    rtc_alarm_event_display(rtc_alarm_event_head);
}

void setup() {
   
    Serial.begin(115200);
    while (!Serial);
    pinMode(LED , OUTPUT); 

    // Serial.println(sizeof(actuator_state));
    // configure_input();
    // rtc_init();
    //pinMode(LED, OUTPUT);
}

void loop() {
    // Not used in FreeRTOS
}


void configure_input(void)
{
    /* Set the GPIO as a push/pull output */
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    // create a queue to handle gpio event from isr
    //gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    //xTaskCreatePinnedToCore(gpio_task_example, "gpio_task_example", 512, NULL, configMAX_PRIORITIES, NULL, 0);
    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // hook isr handler for specific gpio pin
    gpio_isr_handler_add((gpio_num_t)CLINT, gpio_isr_handler, (void *)CLINT);


    // printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());
}


void gpio_isr_handler(void* parameter) {
  
    unsigned int gpio_pin = (unsigned int)parameter;    
    rtc_interpupt_callback(gpio_pin);

    // else if ((cur_time - last_time) / 1000 >= DEBOUNCE_TIME)
    // {
    //     last_time = cur_time;
    //     xQueueSendFromISR(gpio_evt_queue, &gpio_pin, NULL);
    // }
}
