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

xSemaphoreHandle xPassengLockSemaphore;

xSemaphoreHandle xWindowFullUpSemaphore;
xSemaphoreHandle xWindowFullDownSemaphore;


void PrintCurrState(void)
{

	if(State == CentManualOpening)
		UARTprintf("STATE: CentManualOpening \n");
	else if(State == CentManualClosing)
		UARTprintf("STATE: CentManualClosing \n");
	else if(State == PassManualOpening)
		UARTprintf("STATE: PassManualOpening \n");
	else if(State == PassManualClosing)
		UARTprintf("STATE: PassManualClosing \n");
	else if(State == CentAutoOpening)
		UARTprintf("STATE: CentAutoOpening \n");
	else if(State == CentAutoClosing)
		UARTprintf("STATE: CentAutoClosing \n");
	else if(State == PassAutoOpening)
		UARTprintf("STATE: PassAutoOpening \n");
	else if(State == PassAutoClosing)
		UARTprintf("STATE: PassAutoClosing \n");
	else if(State == FullyClosed)
		UARTprintf("STATE: FullyClosed \n");
	else if(State == FullyOpened)
		UARTprintf("STATE: FullyOpened \n");
	else if(State == Neutral)
		UARTprintf("STATE: Neutral \n");
	else
		UARTprintf("STATE: NULL \n");
}

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void
CheckLockSwitch(void)
{
    if( GPIOPinRead(Lock_GPIO_PORT_BASE,LockSwitchPin) != 0) //Reads switch on/off, in case switch already was on/off during the tiva booting up
    {
				if(passLocked == false)
					return; //already false, multiple interrupt, return

        passLocked = false;
        UARTprintf("Passenger Buttons Unlocked\n");
        LCD_print_string("Passeng Unlock");

    }
    else
    {
				if(passLocked == true)
					return; //already true, multiple interrupt, return
					
        passLocked = true;
        UARTprintf("Passenger Buttons Locked\n");
        LCD_print_string("Passeng Locked");

        if (State == PassManualOpening || State == PassManualClosing
                || State == PassAutoOpening || State == PassAutoClosing)
        {
            Force_Window_Stop();
        }
    }
}

////////

void
LockSwitchTask (void * pvParameters) {

    xSemaphoreTake(xPassengLockSemaphore, 0);

    while(1) {

        xSemaphoreTake(xPassengLockSemaphore, portMAX_DELAY);

        if(!bEngineStarted)
        {
            bCentralBtnDebounceReady = true;
            continue; //BLOCK AGAIN //AVOID USING RETURN
        }

        Delay_ms(50);

        CheckLockSwitch();
				
        Delay_ms(50);
				
				bCentralBtnDebounceReady = true;
    }

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LimitSwitchTop(void) {
    State = FullyClosed;
    Force_Window_Stop();
    UARTprintf("Fully Closed Window..\n");
    LCD_print_string("Window closed..");
}

void LimitSwitchBottom(void) {
    State = FullyOpened;
    Force_Window_Stop();
    UARTprintf("Fully Opened Window..\n");
    LCD_print_string("Window opened..");
}

///

void
xWindowFullDownTask (void * pvParameters) {

   xSemaphoreTake(xWindowFullDownSemaphore, 0);

    while(1) {

        xSemaphoreTake(xWindowFullDownSemaphore, portMAX_DELAY);

        LimitSwitchBottom();
						
        Delay_ms(300);

        bCentralBtnDebounceReady = true;
						
						}

}

void
xWindowFullUpTask (void * pvParameters) {

 xSemaphoreTake(xWindowFullUpSemaphore, 0);

    while(1) {

        xSemaphoreTake(xWindowFullUpSemaphore, portMAX_DELAY);

        LimitSwitchTop();
				
        Delay_ms(300);

        bCentralBtnDebounceReady = true;
						
						}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CentralBtnUpPress(void) {

		if(State == FullyClosed)
        {
           UARTprintf("Already Full Closed");
           return;
        }

    State = CentManualClosing;
    UARTprintf("Central: Window closing..\n");
    Force_Window_Up();
    LCD_print_string("Window closing..");
}

void PassengerBtnUpPress(void) {

		if(passLocked)
		{
						UARTprintf("PASSENGER LOCKED");
            return;
		}
		
		if(State == FullyClosed)
        {
           UARTprintf("Already Full Closed");
           return;
        }
				
		if(State == CentManualOpening || State == CentManualClosing )
        {
           UARTprintf("Central Has More Priority");
           return;
					 }
				
    State = PassManualClosing;
    UARTprintf("Passenger: Window closing..\n");
    Force_Window_Up();
    LCD_print_string("Window closing..");
}

/////////////////////////////////////

void CentralBtnDownPress(void) {

		if(State == FullyOpened)
        {
           UARTprintf("Already Full Opened");
           return;
        }
				
    State = CentManualOpening;
    UARTprintf("Central: Window opening..\n");
    Force_Window_Down();
    LCD_print_string("Window opening..");
}

void PassengerBtnDownPress(void) {

		if(passLocked)
		{
						UARTprintf("PASSENGER LOCKED");
            return;
		}

		if(State == FullyOpened)
        {
           UARTprintf("Already Full Opened");
           return;
        }
				
		if(State == CentManualOpening || State == CentManualClosing )
        {
           UARTprintf("Central Has More Priority");
           return;
        }
		
    State = PassManualOpening;
    UARTprintf("Passenger: Window opening..\n");
    Force_Window_Down();
    LCD_print_string("Window opening..");
}

/////////////////////////////////////

void CentBtnRelease(void) {
				
		if( State == FullyClosed || State == FullyOpened ) //To not overwrite these states
			return;

    State = Neutral;
    UARTprintf("Window neutral\n");
    Force_Window_Stop();
    LCD_print_string("Window neutral");
}

void PassBtnRelease(void) {
				
		if( State == FullyClosed || State == FullyOpened ) //To not overwrite these states
			return;
			
		if(State == CentManualOpening || State == CentManualClosing )
        {
           return;
        }

    State = Neutral;
    UARTprintf("Window neutral\n");
    Force_Window_Stop();
    LCD_print_string("Window neutral");
}

/////////////////////////////////////

void CentAutoOpen(void) {
    State = CentAutoOpening;
    UARTprintf("Central AUTO open window\n");
    LCD_print_string("AUTO open window");
}

void CentAutoClose(void) {
    State = CentAutoClosing;
    UARTprintf("Central AUTO open window\n");
    LCD_print_string("AUTO open window");
}


void PassAutoOpen(void) {
    State = PassAutoOpening;
    UARTprintf("Passenger AUTO open window\n");
    LCD_print_string("AUTO open window");
}

void PassAutoClose(void) {
    State = PassAutoClosing;
    UARTprintf("Passenger AUTO close window\n");
    LCD_print_string("AUTO close window");
}

//////////////////////////////////

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
            CentBtnRelease();
        }

        centralBtnUpPressed = !centralBtnUpPressed;

        Delay_ms(300);

        if ( !androidINT && // If it's not android interrupt
                State == CentManualClosing && //If current state is central manual closing
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,CentralBtnUpPin) == 0) //Read if BtnUp is not pressed anymore, therefore released quickly in 300ms
        {
            CentAutoClose();
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
            CentBtnRelease();
        }

        centralBtnDownPressed = !centralBtnDownPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == CentManualOpening &&
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,CentralBtnDownPin) == 0) //Read if BtnDown not pressed anymore
        {
            CentAutoOpen();
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
            PassBtnRelease();
        }

        passengerBtnUpPressed = !passengerBtnUpPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == PassManualClosing &&
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,PassengerBtnUpPin) == 0) //Read if BtnDown not pressed anymore
        {
            PassAutoClose();
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
            PassBtnRelease();
        }

        passengerBtnDownPressed = !passengerBtnDownPressed;

        Delay_ms(300);

        if ( !androidINT &&
                State == PassManualOpening &&
                GPIOPinRead(PowerBTNS_GPIO_PORT_BASE,PassengerBtnDownPin) == 0) //Read if BtnDown not pressed anymore
        {
            PassAutoOpen();
        }

        bCentralBtnDebounceReady = true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    xPassengLockSemaphore = xSemaphoreCreateMutex();
		
		xWindowFullUpSemaphore  = xSemaphoreCreateMutex();
    xWindowFullDownSemaphore  = xSemaphoreCreateMutex();
}

uint32_t
statesTasksInit(void) {

    if (xTaskCreate(PassManualUpTask, (const portCHAR * )
                    "Pass Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }

    if (xTaskCreate(PassManualDownTask, (const portCHAR * )
                    "Pass Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }


    if (xTaskCreate(CentManualUpTask, (const portCHAR * )
                    "Cent Manual Up",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate

    }

    if (xTaskCreate(CentManualDownTask, (const portCHAR * )
                    "Cent Manual Down",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }

    if (xTaskCreate(LockSwitchTask, (const portCHAR * )
                    "LockSwitchTask",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }
		
		 if (xTaskCreate(xWindowFullDownTask, (const portCHAR * )
                    "xWindowFullDownTask",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }
		
		 if (xTaskCreate(xWindowFullUpTask, (const portCHAR * )
                    "xWindowFullDownTask",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    5, NULL) != pdTRUE) {
        return (1);//failed to TaskCreate
    }

    State = Neutral;

    ButtonsInit();
    semaphoresInit();
    return(0); // TaskCreate Success
}

