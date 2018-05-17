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

enum STATE State;

volatile bool centralBtnUpPressed;
volatile bool centralBtnDownPressed;

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
void Force_Window_Down(void) {
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, 0);
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, MotorPin2);
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPinEN, MotorPinEN);
}
void Force_Window_Stop(void) {
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPinEN | MotorPin1 | MotorPin2, 0);
}

//////////////////////////////////////////////////////////

void CentralBtnUpPress(void) {

    State = CentManualClosing;

    UARTprintf("Window closing..\n");

    RedLEDOn();

    Force_Window_Up();

    LCD_print_string("Window closing..");
}

void CentralBtnUpRelease(void) {

    State = Neutral;

    UARTprintf("Window neutral\n");

    RedLEDOff();

    Force_Window_Stop();

    LCD_print_string("Window neutral");
}

void AutoUp(void) {

    State = CentAutoClosing;

    UARTprintf("AUTO close window\n");

    LCD_print_string("AUTO close");
}

//////////////////////////////////////////////////////////

void CentralBtnDownPress(void) {

    State = CentManualOpening;

    UARTprintf("Window opening..\n");

    RedLEDOn();

    Force_Window_Down();

    LCD_print_string("Window opening..");
}

void CentralBtnDownRelease(void) {

    State = Neutral;

    UARTprintf("Window neutral\n");

    RedLEDOff();

    Force_Window_Stop();

    LCD_print_string("Window neutral");
}

void AutoDown(void) {

    State = CentAutoOpening;

    UARTprintf("AUTO open window\n");

    LCD_print_string("AUTO open");
}

//////////////////////////////////////////////////////////

void
CentManualUpTask (void * pvParameters) {

    xSemaphoreTake(xCentralButtonUpSemaphore, 0);

    while(1) {

        xSemaphoreTake(xCentralButtonUpSemaphore, portMAX_DELAY);

        if(!bEngineStarted)
        {
            bCentralBtnDebounceReady = true;
            continue; //BLOCK AGAIN //AVOID USING RETURN
        }

        if (!centralBtnUpPressed)
        {
            CentralBtnUpPress();
        } else
        {
            CentralBtnUpRelease();
        }

        centralBtnUpPressed = !centralBtnUpPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == CentManualClosing &&
                GPIOPinRead(CentralBTNS_GPIO_PORT_BASE,CentralBtnUpPin) == 0) //Read if BtnDown not pressed anymore
        {
            AutoUp();
        }

        bCentralBtnDebounceReady = true;
    }

}

//////////////////////////////////////////////////////////

void
CentManualDownTask (void * pvParameters) {

    xSemaphoreTake(xCentralButtonDownSemaphore, 0);

    while(1) {

        xSemaphoreTake(xCentralButtonDownSemaphore, portMAX_DELAY);

        if(!bEngineStarted)
        {
            bCentralBtnDebounceReady = true;
            continue; //BLOCK AGAIN //AVOID USING RETURN
        }

        if (!centralBtnDownPressed)
        {
            CentralBtnDownPress();
        } else
        {
            CentralBtnDownRelease();
        }

        centralBtnDownPressed = !centralBtnDownPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == CentManualOpening &&
                GPIOPinRead(CentralBTNS_GPIO_PORT_BASE,CentralBtnDownPin) == 0) //Read if BtnDown not pressed anymore
        {
            AutoDown();
        }

        bCentralBtnDebounceReady = true;
    }

}

//////////////////////////////////////////////////////////

void
semaphoresInit(void) {

    xCentralButtonUpSemaphore = xSemaphoreCreateMutex();
    xCentralButtonDownSemaphore = xSemaphoreCreateMutex();

}


uint32_t
statesTasksInit(void) {

    /*

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
    	*/

    if (xTaskCreate(CentManualUpTask, (const portCHAR * )
                    "Cent Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    4, NULL) != pdTRUE) {
        //failed to TaskCreate
        return (1);

    }

    if (xTaskCreate(CentManualDownTask, (const portCHAR * )
                    "Cent Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    4, NULL) != pdTRUE) {
        //failed to TaskCreate
        return (1);
    }
    /*
    if (xTaskCreate(EmergencyTask, (const portCHAR * )
          "Emergency",
          SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
          7, NULL) != pdTRUE) {
    return (1);
    }*/


    centralBtnUpPressed = false;
    centralBtnDownPressed = false;

    State = Neutral;

    ButtonsInit();
    semaphoresInit();
    return(0);
}

