#include "lcdMenu.h"
/* Write by Phuc Phan Hoang*/
Menu MainMenu = {
    NULL,
    3,1,
    "   MAIN MENU      ",
    {
    "   SENSOR         ",
    "   ACTUATOR       ",
    "   SETTING        ",
    ""
    },

    {
    &SensorMenu,
    &ActuatorMenu,
    NULL,
    NULL
    },
    NULL,NULL,
    NULL
};

// Sensor menu
Menu SensorMenu = {
    &MainMenu,
    2,1,
    "   SENSOR MENU    ",
    {
    "   WATER          ",
    "   LAND           ",
    "",
    ""
    },
    {
    &WaterSensorMenu,
    &LandSensorMenu,
    NULL,
    NULL
    },
    NULL,NULL,
    NULL
};

// Water sensor menu
Menu WaterSensorMenu = {
    &SensorMenu,
    2,1,
    "   WATER SENSOR   ",
    {
    "   WATER LEVEL    ",
    "   TEMP           ",
    "",
    ""
    },
    {NULL,NULL,NULL,NULL},
    NULL,NULL,
    &SensorDisplay
};

// land sensor menu
Menu LandSensorMenu = {
    &SensorMenu,
    4,1,
    "   LAND SENSOR    ",
    {
    "   TEMP AND HUM   ",
    "   PH             ",
    "   MOISTURE       ",
    "   LIGHT          "
    },
    {NULL,NULL,NULL,NULL},

    NULL,NULL,
    &SensorDisplay
};
// Show data of sensor
Menu SensorShowInfo = 
{
    NULL,
    1,1,
    "",
    {
    "",
    "",
    "",
    ""
    },
    {NULL,NULL,NULL,NULL},
    NULL,NULL,
    NULL
};
// Actuator sensor menu
Menu ActuatorMenu = {
    &MainMenu,
    3,1,
    "   ACTUATOR       ",
    {
    "   RELAY          ",
    "   SERVO          ",
    "   LED            ",
    ""
    },
    {
        &RelayMenu,
        &ServoMenu,
        &LedMenu,
        NULL
    },
    NULL,NULL,
    NULL
};


Menu RelayMenu = {
    &ActuatorMenu,
    2,1,
    "   RELAY          ",
    {
    "   ON             ",
    "   OFF            ",
    "",
    ""
    },
    {NULL,NULL,NULL,NULL},
    &ActuatorsActivation,&ActuatorsActivation,
    NULL
};

Menu ServoMenu = {
    &ActuatorMenu,
    2,1,
    "   SERVO          ",
    {
    "   ON             ",
    "   OFF            ",
    "",
    "",
    },
    {NULL,NULL,NULL,NULL},
    &ActuatorsActivation,&ActuatorsActivation,
    NULL
};

Menu LedMenu = {
    &ActuatorMenu,
    2,1,
    "   LED          ",
    {
    "   ON             ",
    "   OFF            ",
    "",
    ""
    },
    {NULL,NULL,NULL,NULL},
    &ActuatorsActivation,&ActuatorsActivation,
    NULL
};


__attribute__((weak)) void MenuInit(void)
{
    // display.begin(i2c_Address, true); // Address 0x3C default
    // display.display();
    // display.clearDisplay();
    // display.setTextSize(1);
    // display.setTextColor(SH110X_WHITE);
    // MenuDisplay(&MainMenu,MainMenu.cur_cursor);
}

__attribute__((weak)) void MenuDisplay(Menu *menu, uint8_t select)
{
    // display.clearDisplay();
    // display.setCursor(0, 0);
    // display.println(menu->Title);
    // for(int i = 0; i < menu->maxSelect;i++)
    // {
    //     display.setCursor(0, 12*(i+1));
    //     display.println(menu->MenuList[i]);
    // }   
    // display.drawBitmap(0, 12*select,  logo8_cursor, 8, 8, 1);
    
    // display.display();
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

__attribute__((weak)) void ActuatorsActivation(char Device, char State)
{
    (void)Device;
    (void)State;
}

__attribute__((weak)) void SensorDisplay(char Device, char State)
{
    (void)Device;
    (void)State;
}