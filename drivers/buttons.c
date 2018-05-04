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

//test

#include "driverlib/pwm.h"
////////////////////////////

#define SWITCHTASKSTACKSIZE        128 

////////////////////Central Buttons Port and Pins
#define CentralBTNS_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOA
#define CentralBTNS_GPIO_PORT_BASE GPIO_PORTA_BASE 
#define CentralBtn1pin GPIO_PIN_6
#define CentralBtn2pin GPIO_PIN_7
//////////////////////

//trying with PF0 and PF4 for Central Buttons
/*#define CentralBTNS_GPIO_PORT_BASE GPIO_PORTF_BASE 
#define CentralBtn1pin GPIO_PIN_0
#define CentralBtn2pin GPIO_PIN_4*/


////////////////////Motor Port and Pins
#define Motor_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOA
#define Motor_GPIO_PORT_BASE GPIO_PORTA_BASE 
#define MotorPin1 GPIO_PIN_2
#define MotorPin2 GPIO_PIN_3
//////////////////////


volatile bool CentralBtn1pin_Pressed = false;
volatile bool CentralBtn2pin_Pressed = false;

//extern xSemaphoreHandle xButtonPressedSemaphore;
//extern xQueueHandle g_pLEDQueue;

void onButtonInt(void) {

		//Get which pin interrupted
		uint8_t INT_PIN_NUM = GPIOIntStatus(CentralBTNS_GPIO_PORT_BASE, false);

		if (INT_PIN_NUM & CentralBtn1pin) //if INT_PIN_NUM == CentralBtn1pin
		{
		
			if (!CentralBtn1pin_Pressed) //If btn not held down ( Btn pressed )
			{
				 UARTprintf("CentralBtn1 Down \n");
				 //turn on led
				 GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 2);
				 //turn on motor
				 GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, MotorPin1);
				 GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, 0);
			}
			else // btn was held down ( Btn released )
			{
					UARTprintf("CentralBtn1 Up \n");
					//turn off led
					GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0 );
					//turn off motor
				  GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, 0);
				  GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, 0);
      }
				CentralBtn1pin_Pressed = !CentralBtn1pin_Pressed; //flip pressed bool 
				GPIOIntClear(CentralBTNS_GPIO_PORT_BASE, INT_PIN_NUM);  // Clear interrupt flag
				
		}
		else if (INT_PIN_NUM & CentralBtn2pin)
		{
		
			if (!CentralBtn2pin_Pressed)
			{
				 UARTprintf("CentralBtn2 Down \n");
				 //turn on led
				 GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 , 2);
				 //turn on motor
				 GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, 0);
				 GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, MotorPin2);
			}
			else
			{
				 UARTprintf("CentralBtn2 Up \n");
				 //turn off led
				 GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0 );
				 //turn off motor
				 GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, 0);
				 GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, 0);
			}
				 CentralBtn2pin_Pressed = !CentralBtn2pin_Pressed;
				 GPIOIntClear(CentralBTNS_GPIO_PORT_BASE, INT_PIN_NUM);  // Clear interrupt flag
				 
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
//		uint16_t PWM_FREQUENCY = 400;
//		uint16_t ui8Adjust = 440;
		
		
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

		//enable led for testing
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
		
		//enable entral Buttons pins, CentralBTNS_GPIO_PORT_BASE and pin numbers are defined at the start of this doc ////////////////////////////////////
    ROM_SysCtlPeripheralEnable(CentralBTNS_SYSCTL_PERIPH_GPIO); //comment this line out if you're trying PF0 and PF4
		GPIOPinTypeGPIOInput(CentralBTNS_GPIO_PORT_BASE, CentralBtn1pin|CentralBtn2pin); //comment this line out if you're trying PF0 and PF4
		//Central Buttons INTERRUPT INIT //////////////////////////////////
		GPIOIntDisable(CentralBTNS_GPIO_PORT_BASE, CentralBtn1pin);
		GPIOIntDisable(CentralBTNS_GPIO_PORT_BASE, CentralBtn2pin);
    GPIOIntTypeSet(CentralBTNS_GPIO_PORT_BASE, CentralBtn1pin, GPIO_BOTH_EDGES);
		GPIOIntTypeSet(CentralBTNS_GPIO_PORT_BASE, CentralBtn2pin, GPIO_BOTH_EDGES);
		GPIOIntEnable(CentralBTNS_GPIO_PORT_BASE, CentralBtn1pin);
		GPIOIntEnable(CentralBTNS_GPIO_PORT_BASE, CentralBtn2pin);
		////////////////////////////////////////////////////////////////////
		GPIOIntRegister(CentralBTNS_GPIO_PORT_BASE, onButtonInt);
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		 
		//enable motor pins
		GPIOPinTypeGPIOOutput(Motor_GPIO_PORT_BASE, MotorPin1);
		GPIOPinTypeGPIOOutput(Motor_GPIO_PORT_BASE, MotorPin2);
		GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin1, 0);
		GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPin2, 0);
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
