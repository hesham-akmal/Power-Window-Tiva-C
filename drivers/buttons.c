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
#include "utils/uartstdio.h"
#include "PORTS.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "priorities.h"
#include "driverlib/interrupt.h"
#include "LCD.h"
#include "states_tasks.h"


////////////////////////////

#define SWITCHTASKSTACKSIZE 128

extern xSemaphoreHandle xCentralButtonUpSemaphore;
extern xSemaphoreHandle xCentralButtonDownSemaphore;
extern xSemaphoreHandle xEngineStartButtonPressedSemaphore;

bool bCentralBtnDebounceReady;
bool bEngineStarted;

void UnblockTaskWithSemaphore(xSemaphoreHandle x)
{
    portBASE_TYPE xHigherPTW = pdFALSE;
    xSemaphoreGiveFromISR(x, & xHigherPTW);
    portEND_SWITCHING_ISR(xHigherPTW);
}

void onButtonInt(void) {

    //Get which pin interrupted
    uint32_t INT_PIN_NUM = GPIOIntStatus(CentralBTNS_GPIO_PORT_BASE, false);

    GPIOIntClear(CentralBTNS_GPIO_PORT_BASE, INT_PIN_NUM); // Clear interrupt flag

    if (INT_PIN_NUM & CentralBtnDownPin)
    {

        if(!bCentralBtnDebounceReady) //for debouncing central button //If not ready to listen to button change, return.
            return;

        bCentralBtnDebounceReady = false; //If ready, set it to false, until Switch Task sets it to true again.

        UnblockTaskWithSemaphore(xCentralButtonDownSemaphore);

    } else if (INT_PIN_NUM &  CentralBtnUpPin) {

        if(!bCentralBtnDebounceReady) //for debouncing central button //If not ready to listen to button change, return.
            return;

        bCentralBtnDebounceReady = false; //If ready, set it to false, until Switch Task sets it to true again.

        UnblockTaskWithSemaphore(xCentralButtonUpSemaphore);

    } else if (INT_PIN_NUM & EngineStartButton) {

        UnblockTaskWithSemaphore(xEngineStartButtonPressedSemaphore);

    }
}

void
onPassButtonInt (void){
		//Get which pin interrupted
    INT_PIN_NUM = GPIOIntStatus(PowerBTNS_GPIO_PORT_BASE, false);
		
		GPIOIntClear(PowerBTNS_GPIO_PORT_BASE, INT_PIN_NUM); // Clear interrupt flag
		
		if (State == Neutral){

		//The following if statement is needed, as when a button is pressed, multiple strange interrupts could occur.
		//We only need certain pin interrupts
		
			if (INT_PIN_NUM & PassengerBtnDownPin){
					
				if(!bCentralBtnDebounceReady) //for debouncing central button //If not ready to listen to button change, return.
					return;
					
				bCentralBtnDebounceReady = false; //If ready, set it to false, until Switch Task sets it to true again.
			
				portBASE_TYPE xHigherPTW = pdFALSE;
				xSemaphoreGiveFromISR(xPassengerButtonDownSemaphore, & xHigherPTW);
				portEND_SWITCHING_ISR(xHigherPTW);
				return;
			
		} else if (INT_PIN_NUM &  PassengerBtnUpPin){
			
				if(!bCentralBtnDebounceReady) //for debouncing central button //If not ready to listen to button change, return.
					return;
					
				bCentralBtnDebounceReady = false; //If ready, set it to false, until Switch Task sets it to true again.
			
				portBASE_TYPE xHigherPTW = pdFALSE;
				xSemaphoreGiveFromISR(xPassengerButtonUpSemaphore, & xHigherPTW);
				portEND_SWITCHING_ISR(xHigherPTW);
				return;
		}
	}
}

void
ButtonsInit(void) {
    //*************************************************************************************************************
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
    //************************************************************************************************************


    //enable led for testing
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);


    //enable Central Buttons pins, CentralBTNS_GPIO_PORT_BASE and pin numbers are defined at PORTS.h ////////////////////////////////////
    ROM_SysCtlPeripheralEnable(PowerBTNS_SYSCTL_PERIPH_GPIO);                            //comment this line out if you're trying PF0 and PF4
    GPIOPinTypeGPIOInput(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin); //comment this line out if you're trying PF0 and PF4
    //Central Buttons INTERRUPT INIT //////////////////////////////////
    GPIOIntDisable(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin);
    GPIOIntTypeSet(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin, GPIO_BOTH_EDGES);
    GPIOIntEnable(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin);
    ////////////////////////////////////////////////////////////////////
		
		//enable Passenger Buttons pins, PassengerBTNS_GPIO_PORT_BASE and pin numbers are defined at PORTS.h ////////////////////////////////////
    GPIOPinTypeGPIOInput(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin); //comment this line out if you're trying PF0 and PF4
    //Passenger Buttons INTERRUPT INIT //////////////////////////////////
    GPIOIntDisable(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin);
    GPIOIntTypeSet(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin, GPIO_BOTH_EDGES);
    GPIOIntEnable(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin);
    ////////////////////////////////////////////////////////////////////
		
		
		
    GPIOIntRegister(PowerBTNS_GPIO_PORT_BASE, onButtonInt);	//Link the method that is going to be called on the interrupt
		GPIOIntRegister(PowerBTNS_GPIO_PORT_BASE, onPassButtonInt);	//Link the method that is going to be called on the interrupt
		
		bCentralBtnDebounceReady = true;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Motor INIT ////////////////////////////////////////////////////////////////////////////////////////////
    ROM_SysCtlPeripheralEnable(Motor_SYSCTL_PERIPH_GPIO);
    GPIOPinTypeGPIOOutput(Motor_GPIO_PORT_BASE, MotorPinEN | MotorPin1 | MotorPin2);
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPinEN | MotorPin1 | MotorPin2 , 0);

    //Engine INIT ///////////////////////////////////////////////////////////////////////////////////////////
    GPIOPinTypeGPIOInput(EngineStartButton_GPIO_PORT_BASE, EngineStartButton);
    //LINE BELOW ENSURE PULL UP, MUST BE CALLED AFTER GPIOPinTypeGPIOInput, ELSE IT WON'T WORK
    //MUST BE DONE FOR SMALL SWITCHES AND LIMIT SWITCHES PROBABLY
    GPIOPadConfigSet(EngineStartButton_GPIO_PORT_BASE, EngineStartButton, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //INT INIT
    GPIOIntDisable(EngineStartButton_GPIO_PORT_BASE, EngineStartButton);
    GPIOIntTypeSet(EngineStartButton_GPIO_PORT_BASE, EngineStartButton, GPIO_BOTH_EDGES );
    GPIOIntEnable(EngineStartButton_GPIO_PORT_BASE, EngineStartButton);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
