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
//#include "switch_task.h"
#include "led_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "PORTS.h"
#include "LCD.h"
#include "states_tasks.h"

#define SWITCHTASKSTACKSIZE 128

volatile bool centralBtnUpPressed;

xSemaphoreHandle xCentralButtonUpSemaphore;
xSemaphoreHandle xCentralButtonDownSemaphore;

void RedLEDOn(void) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 2);
}

void RedLEDOff(void) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
}

void Force_Window_Up(void) {
    //Set H Bridge in1 and in2 and EN
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, MotorPin1);
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, 0);
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPinEN, MotorPinEN);
}

void Force_Window_Stop(void) {
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPinEN | MotorPin1 | MotorPin2, 0);
}

void CentralBtnUpPress(void) {
    UARTprintf("Window closing..\n");

    RedLEDOn();

    Force_Window_Up();

    LCD_print_string("Window closing..");
}

void CentralBtnUpRelease(void) {
    UARTprintf("Window neutral\n");

    RedLEDOff();

    Force_Window_Stop();

    LCD_print_string("Window neutral");
}

static void
CentManualUpTask (void * pvParameters){
	
	xSemaphoreTake(xCentralButtonUpSemaphore, 0);
	
	while(1){
		
		xSemaphoreTake(xCentralButtonUpSemaphore, portMAX_DELAY);
		if (!centralBtnUpPressed)
		{
				CentralBtnUpPress();
		} else
		{
				CentralBtnUpRelease();
		}

		centralBtnUpPressed = !centralBtnUpPressed;
		Delay_ms(300);
		bCentralBtnDebounceReady = true;
	}
	
	
	
}

void
semaphoresInit(void){
	
	xCentralButtonUpSemaphore = xSemaphoreCreateMutex();
	xCentralButtonDownSemaphore = xSemaphoreCreateMutex();
	
	centralBtnUpPressed = false;

}	


uint32_t
statesTasksInit(void){
	
	/*if (xTaskCreate(NeutralTask, (const portCHAR * )
                    "Neutral",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    2, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(PassAutoUpTask, (const portCHAR * )
                    "Pass Auto Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(PassAutoDownTask, (const portCHAR * )
                    "Pass Auto Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(PassManualUpTask, (const portCHAR * )
                    "Pass Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    3, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(PassManualDownTask, (const portCHAR * )
                    "Pass Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    3, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(CentAutoUpTask, (const portCHAR * )
                    "Cent Auto Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    6, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(CentAutoDownTask, (const portCHAR * )
                    "Cent Auto Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    6, NULL) != pdTRUE) {
        return (1);
    }*/
										
		if (xTaskCreate(CentManualUpTask, (const portCHAR * )
                    "Cent Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    4, NULL) != pdTRUE) {
        return (1);
    }
										
		/*if (xTaskCreate(CentManualDownTask, (const portCHAR * )
                    "Cent Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    4, NULL) != pdTRUE) {
        return (1);
    }
										
		if (xTaskCreate(EmergencyTask, (const portCHAR * )
                    "Emergency",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    7, NULL) != pdTRUE) {
        return (1);
    }*/
				
		ButtonsInit();
		semaphoresInit();
		return(1);
										
}

