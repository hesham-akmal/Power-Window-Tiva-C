//*****************************************************************************
//
// engine_task.c - A simple switch task to process the engine state.
//


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LCD.h"
#include "PORTS.h"

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define SWITCHTASKSTACKSIZE 128 // Stack size in words

xSemaphoreHandle xEngineStartButtonPressedSemaphore;

//From buttons.c //////////////////////
extern bool bEngineStartDebounceReady;
///////////////////////////////////////
extern void Force_Window_Up(void);


void
EngineOn(void)
{
    bEngineStarted = true;
    LCD_print_string("ENGINE ON");
    UARTprintf("ENGINE ON\n");
}

void
EngineOff(void)
{
    bEngineStarted = false;
    LCD_print_string("ENGINE OFF");
    UARTprintf("ENGINE OFF\n");
		Force_Window_Up();
}

void
CheckEngineSwitch()
{
    //Reads switch on/off pin and apply function
    if( GPIOPinRead(EngineStartButton_GPIO_PORT_BASE,EngineStartButton) == 0 )
        EngineOn();
    else
        EngineOff();
}

void
EngineTask(void * pvParameters) {

    xSemaphoreTake(xEngineStartButtonPressedSemaphore, 0);

    while (1)
    {
        //Block till button interrupt gives semaphore back
        xSemaphoreTake(xEngineStartButtonPressedSemaphore, portMAX_DELAY);

        //When unblocked, check if switch closed or opened
        CheckEngineSwitch();
    }
}

//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t
EngineTaskInit(void) {
    //
    // Create button semaphore
    //
    xEngineStartButtonPressedSemaphore = xSemaphoreCreateMutex();

    CheckEngineSwitch();

    Delay_ms(500);

    CheckLockSwitch();

    //
    // Create the switch task.
    //
    if (xTaskCreate(EngineTask, (const portCHAR * )
                    "Engine",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    PRIORITY_SWITCH_TASK, NULL) != pdTRUE) {
        return (1);
    }

    //
    // Success.
    //
    return (0);
}
