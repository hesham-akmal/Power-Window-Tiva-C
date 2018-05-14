
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
#include "PORTS.h"
#include "LCD.h"


#define SWITCHTASKSTACKSIZE 128 // Stack size in words

extern uint8_t INT_PIN_NUM;
bool androidINT;

void
UnblockSwitchTask(void)
{
    portBASE_TYPE xHigherPTW = pdFALSE;
    xSemaphoreGiveFromISR(xButtonPressedSemaphore, & xHigherPTW);
    portEND_SWITCHING_ISR(xHigherPTW);
}

static void
ListenTask(void * pvParameters) {

    while (1)
    {
        unsigned char command = UARTgetc();

        androidINT = true;

        switch(command)
        {
        case 'c' :
            INT_PIN_NUM = CentralBtnDownPin;
            UnblockSwitchTask();
            break;

        case 'd' :
            INT_PIN_NUM = CentralBtnUpPin;
            UnblockSwitchTask();
            break;

        case 'b' :
            //engine on/off
            break;
        }
    }
}

uint32_t
ListenTaskInit(void) {

    if (xTaskCreate(ListenTask, (const portCHAR * )
                    "Listen",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    PRIORITY_SWITCH_TASK, NULL) != pdTRUE) {
        return (1);
    }

    //
    // Success.
    //
    return (0);
}
