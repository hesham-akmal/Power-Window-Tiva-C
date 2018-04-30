//*****************************************************************************
//
// buttons.c - Evaluation board driver for push buttons.
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
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "drivers/buttons.h"

//for assig2/////////////////
#include "FreeRTOS.h"
#include "priorities.h"
#include "driverlib/interrupt.h" 
#include "switch_task.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "utils/uartstdio.h"
////////////////////////////

#define SWITCHTASKSTACKSIZE        128 

extern xSemaphoreHandle xButtonPressedSemaphore;

//*****************************************************************************
//
//! \addtogroup buttons_api
//! @{
//
//*****************************************************************************

extern xQueueHandle g_pLEDQueue;

bool PIN_0_Pressed = false;
bool PIN_4_Pressed = false;

void onButtonInt(void) {

		//Get which pin interrupted
		uint8_t PIN_NUM = GPIOIntStatus(GPIO_PORTF_BASE, false);

		if (PIN_NUM & GPIO_PIN_0) //PF0 btn CLICK //if PIN_NUM == GPIO_PIN_0
		{
			if (!PIN_0_Pressed) //If btn not held down
			{
				 UARTprintf("PF0 Down \n");
				 //turn on led
				 GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 2);
				 //turn on motor
				 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2);
			}
			else // btn was held down
			{
					UARTprintf("PF0 Up \n");
					//turn off led
					GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0 );
					//turn off motor?
					GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0);
      }
				PIN_0_Pressed = !PIN_0_Pressed; //flip pressed bool 
				GPIOIntClear(GPIO_PORTF_BASE, PIN_NUM);  // Clear interrupt flag
		}
		
		else if (PIN_NUM & GPIO_PIN_4) //PF4 btn CLICK
		{
			if (!PIN_4_Pressed)
			{
				 UARTprintf("PF4 Down \n");
				 //turn on led
				 GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 2);
			}
			else
			{
				 UARTprintf("PF4 Up \n");
				 //turn off led
				 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0 );
			}
				 PIN_4_Pressed = !PIN_4_Pressed;
				 GPIOIntClear(GPIO_PORTF_BASE, PIN_NUM);  // Clear interrupt flag
		}
		
}

//*****************************************************************************
//
//! Initializes the GPIO pins used by the board pushbuttons.
//!
//! This function must be called during application initialization to
//! configure the GPIO pins to which the pushbuttons are attached.  It enables
//! the port used by the buttons and configures each button GPIO as an input
//! with a weak pull-up.
//!
//! \return None.
//
//*****************************************************************************

void
ButtonsInit(void)
{
    //
    // Enable the GPIO port to which the pushbuttons are connected.
    //
    ROM_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);

    //
    // Unlock PF0 so we can change it to a GPIO input
    // Once we have enabled (unlocked) the commit register then re-lock it
    // to prevent further changes.  PF0 is muxed with NMI thus a special case.
    //
    HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) |= 0x01;
    HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0;

    //
    // Set each of the button GPIO pins as an input with a pull-up.
    //
    ROM_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

		//DISABLE INTERRUPTS AT INIT
		GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0);
		GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);
		// To set interrupts 
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES);
		// ENABLE INTERRUPTS
		GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0);
		GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);

		//Set Handler to onButtonInt method
		GPIOIntRegister(GPIO_PORTF_BASE, onButtonInt);
		
		//enable led for testing
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
		
		//enable motor pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
