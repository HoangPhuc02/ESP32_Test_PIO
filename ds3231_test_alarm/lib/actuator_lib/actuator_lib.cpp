#include "actuator_lib.h"


/*==================== Actuator Function ===================*/
void actuator_init(void)
{
    /* update data from uart transfer*/
    actuator_config(&actuators[0], Relay, ACTUATOR_OFF);        
    actuator_config(&actuators[1], Servo, ACTUATOR_OFF),
    actuator_config(&actuators[2], Led, ACTUATOR_OFF),
    actuator_control_queue = xQueueCreate(10, sizeof(Actuator_Init_t));
}
void actuator_config(Actuator_Init_t *actuator, Actuator_Type_t type, Actuator_State_t state)
{
    actuator->ActuatorId = type;
    actuator->ActuatorState = state;
}
void set_state_actuator(Actuator_Init_t *actuator, Actuator_State_t state)
{
    if (actuator->ActuatorState != state)
        actuator->ActuatorState = state;
}
Actuator_State_t get_state_actuator(Actuator_Init_t *actuator)
{
    return actuator->ActuatorState;
}
/*==========================================================*/
