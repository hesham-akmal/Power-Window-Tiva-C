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

volatile bool centralBtnUpPressed;
volatile bool centralBtnDownPressed;
volatile bool passengerBtnUpPressed;
volatile bool passengerBtnDownPressed;

enum STATE State;

xSemaphoreHandle xCentralButtonUpSemaphore;
xSemaphoreHandle xCentralButtonDownSemaphore;

xSemaphoreHandle xPassengerButtonUpSemaphore;
xSemaphoreHandle xPassengerButtonDownSemaphore;

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
    UARTprintf("Central: Window closing..\n");
    Force_Window_Up();
    LCD_print_string("Window closing..");
}

void PassengerBtnUpPress(void) {
    State = PassManualClosing;
    UARTprintf("Passenger: Window closing..\n");
    Force_Window_Up();
    LCD_print_string("Window closing..");
}

//////////////////////////////////////////////////////////

void CentralBtnDownPress(void) {
    State = CentManualOpening;
    UARTprintf("Central: Window opening..\n");
    Force_Window_Down();
    LCD_print_string("Window opening..");
}

void PassengerBtnDownPress(void) {
    State = PassManualOpening;
    UARTprintf("Passenger: Window opening..\n");
    Force_Window_Down();
    LCD_print_string("Window opening..");
}

//////////////////////////////////////////////////////////

void PowerBtnRelease(void) { //for central/passenger up/down
    State = Neutral;
    UARTprintf("Window neutral\n");
    Force_Window_Stop();
    LCD_print_string("Window neutral");
}

//////////////////////////////////////////////////////////

void AutoOpen(void) {
    State = AutoOpening;
    UARTprintf("AUTO open window\n");
    LCD_print_string("AUTO open window");
}

void AutoClose(void) {
    State = AutoClosing;
    UARTprintf("AUTO close window\n");
    LCD_print_string("AUTO close window");
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
            PowerBtnRelease();
        }

        centralBtnUpPressed = !centralBtnUpPressed;

        Delay_ms(300);

        if ( !androidINT && // If it's not android interrupt
                State == CentManualClosing && //If current state is central manual closing
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,CentralBtnUpPin) == 0) //Read if BtnUp is not pressed anymore, therefore released quickly in 300ms
        {
            AutoClose();
        }

        bCentralBtnDebounceReady = true;
    }

}

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
            PowerBtnRelease();
        }

        centralBtnDownPressed = !centralBtnDownPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == CentManualOpening &&
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,CentralBtnDownPin) == 0) //Read if BtnDown not pressed anymore
        {
            AutoOpen();
        }

        bCentralBtnDebounceReady = true;
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////


void
PassManualUpTask (void * pvParameters) {

    xSemaphoreTake(xPassengerButtonUpSemaphore, 0);

    while(1) {

        xSemaphoreTake(xPassengerButtonUpSemaphore, portMAX_DELAY);

        if(!bEngineStarted)
        {
            bCentralBtnDebounceReady = true;
            continue; //BLOCK AGAIN //AVOID USING RETURN
        }

        if (!passengerBtnUpPressed)
        {
            PassengerBtnUpPress();
        } else
        {
            PowerBtnRelease();
        }

        passengerBtnUpPressed = !passengerBtnUpPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == PassManualClosing &&
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,PassengerBtnUpPin) == 0) //Read if BtnDown not pressed anymore
        {
            AutoClose();
        }

        bCentralBtnDebounceReady = true;
    }
}


void
PassManualDownTask (void * pvParameters) {

    xSemaphoreTake(xPassengerButtonDownSemaphore, 0);

    while(1) {

        xSemaphoreTake(xPassengerButtonDownSemaphore, portMAX_DELAY);

        if(!bEngineStarted)
        {
            bCentralBtnDebounceReady = true;
            continue; //BLOCK AGAIN //AVOID USING RETURN
        }

        if (!passengerBtnDownPressed)
        {
            PassengerBtnDownPress();
        } else
        {
            PowerBtnRelease();
        }

        passengerBtnDownPressed = !passengerBtnDownPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == PassManualOpening &&
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,PassengerBtnDownPin) == 0) //Read if BtnDown not pressed anymore
        {
            AutoOpen();
        }

        bCentralBtnDebounceReady = true;
    }
}

//////////////////////////////////////////////////////////

void
semaphoresInit(void) {

    xCentralButtonUpSemaphore = xSemaphoreCreateMutex();
    xCentralButtonDownSemaphore = xSemaphoreCreateMutex();
    centralBtnUpPressed = false;
    centralBtnDownPressed = false;

    xPassengerButtonUpSemaphore = xSemaphoreCreateMutex();
    xPassengerButtonDownSemaphore = xSemaphoreCreateMutex();
    passengerBtnUpPressed = false;
    passengerBtnDownPressed = false;
		
}

uint32_t
statesTasksInit(void) {

    	if (xTaskCreate(PassManualUpTask, (const portCHAR * )
                    "Pass Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    3, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }

    	if (xTaskCreate(PassManualDownTask, (const portCHAR * )
                    "Pass Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    3, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }
    	

    if (xTaskCreate(CentManualUpTask, (const portCHAR * )
                    "Cent Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    4, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate

    }

    if (xTaskCreate(CentManualDownTask, (const portCHAR * )
                    "Cent Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    4, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }
		
    /*
    if (xTaskCreate(EmergencyTask, (const portCHAR * )
          "Emergency",
          SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
          7, NULL) != pdTRUE) {
    return (1);
    }*/

    State = Neutral;

    ButtonsInit();
    semaphoresInit();
    return(0); // TaskCreate Success
}

