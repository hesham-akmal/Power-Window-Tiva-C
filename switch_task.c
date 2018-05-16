//*****************************************************************************
//
// switch_task.c - A simple switch task to process the buttons.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************


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



//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define SWITCHTASKSTACKSIZE 128 // Stack size in words

xSemaphoreHandle xButtonPressedSemaphore;

volatile bool bCentralBtnDownPressed;
volatile bool bCentralBtnUpPressed;
//From buttons.c //////////////////////
extern volatile bool bEngineStarted;

void RedLEDOn(void) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 2);
}

void RedLEDOff(void) {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
}

//

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

//

void CentralBtnDownPress(void) {
    UARTprintf("Window opening..\n");

    RedLEDOn();

    Force_Window_Down();

    LCD_print_string("Window opening..");
}

void AutoDown(void) {
    UARTprintf("AUTO open window\n");

    LCD_print_string("AUTO open");
}

void CentralBtnDownRelease(void) {
    UARTprintf("Window neutral\n");

    RedLEDOff();

    Force_Window_Stop();

    LCD_print_string("Window neutral");
}

//

void CentralBtnUpPress(void) {
    UARTprintf("Window closing..\n");

    RedLEDOn();

    Force_Window_Up();

    LCD_print_string("Window closing..");
}

void AutoUp(void) {
    UARTprintf("AUTO close window\n");

    LCD_print_string("AUTO close");
}

void CentralBtnUpRelease(void) {
    UARTprintf("Window neutral\n");

    RedLEDOff();

    Force_Window_Stop();

    LCD_print_string("Window neutral");
}


//*****************************************************************************
//
// This task handles central button behaviour
//
//*****************************************************************************
static void
SwitchTask(void * pvParameters) {

    xSemaphoreTake(xButtonPressedSemaphore, 0);
    bool bCentralAutoDownCheck = false;
    bool bCentralAutoUpCheck = false;

    while (1)
    {
        //Block till button interrupt gives semaphore back
        xSemaphoreTake(xButtonPressedSemaphore, portMAX_DELAY);

				if(!bEngineStarted)
					return;

        if (INT_PIN_NUM & CentralBtnDownPin) //if INT_PIN_NUM == CentralBtnDownPin
        {

            if (!bCentralBtnDownPressed) //If btn was not held down, therefore it's pressed
            {
                CentralBtnDownPress();
                bCentralAutoDownCheck = true;
            } else // btn was already held down
            {
                CentralBtnDownRelease();
            }

            bCentralBtnDownPressed = !bCentralBtnDownPressed; //flip pressed bool

        } else if (INT_PIN_NUM & CentralBtnUpPin) {

            if (!bCentralBtnUpPressed)
            {
                CentralBtnUpPress();
                bCentralAutoUpCheck = true;
            } else
            {
                CentralBtnUpRelease();
            }

            bCentralBtnUpPressed = !bCentralBtnUpPressed;

        }
        // write here if conditions for limit switches



        //wait till the button behavior is checked again in the buttons.c intterupt method onButtonInt()
        Delay_ms(300);

        if(!androidINT) ////not working for some reason
        {
            if (bCentralAutoDownCheck)
            {
                if (GPIOPinRead(CentralBTNS_GPIO_PORT_BASE,CentralBtnDownPin) == 0) //Read if BtnDown still pressed
                {
                    AutoDown();
                }

                bCentralAutoDownCheck = false;
            }
            else if (bCentralAutoUpCheck)
            {
                if (GPIOPinRead(CentralBTNS_GPIO_PORT_BASE,CentralBtnUpPin) == 0) //Read if BtnUp still pressed
                {
                    AutoUp();
                }

                bCentralAutoUpCheck = false;
            }


            bCentralBtnDebounceReady = true;
        }
        else
        {
            androidINT = false; //reset
        }
    }
}

//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t
SwitchTaskInit(void) {

    //
    // Create button semaphore
    //
    xButtonPressedSemaphore = xSemaphoreCreateMutex();

    //
    // Initialize the buttons
    //
    ButtonsInit();

    bCentralBtnDownPressed = false;
    bCentralBtnUpPressed = false;
		androidINT = false;
    LCD_print_string("Window neutral");

    //
    // Create the switch task.
    //
    if (xTaskCreate(SwitchTask, (const portCHAR * )
                    "Switch",
                    SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                    PRIORITY_SWITCH_TASK, NULL) != pdTRUE) {
        return (1);
    }

    //
    // Success.
    //
    return (0);
}
