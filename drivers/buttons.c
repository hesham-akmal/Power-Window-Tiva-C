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
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "priorities.h"
#include "driverlib/interrupt.h"
#include "LCD.h"
#include "states_tasks.h"
#include "PORTS.h"

#define SWITCHTASKSTACKSIZE 128

////////////////////////////////////////

bool bCentralBtnDebounceReady;
bool passLocked;
bool bEngineStarted;

void UnblockTaskWithSemaphore(xSemaphoreHandle x)
{
    portBASE_TYPE xHigherPTW = pdFALSE;
    xSemaphoreGiveFromISR(x, & xHigherPTW);
    portEND_SWITCHING_ISR(xHigherPTW);
}

////////////////////////////////////////////////////////////////////////////

void onLimitSwitchesInt(void) {

    //Get which pin interrupted
    uint32_t INT_PIN_NUM_SWITCH = GPIOIntStatus(Limits_GPIO_PORT_BASE, false);

    GPIOIntClear(Limits_GPIO_PORT_BASE, INT_PIN_NUM_SWITCH); // Clear interrupt flag

    androidINT = false; //not android interrupt
		
		if(!bCentralBtnDebounceReady) //for debouncing central button //If not ready to listen to button change, return.
				return;
		bCentralBtnDebounceReady = false; //If ready, set it to false, until it's set to true again.

    switch(INT_PIN_NUM_SWITCH)
    {
			case WindowUpLimitPin:
					
					UnblockTaskWithSemaphore(xWindowFullUpSemaphore);
					
					break;

			case WindowDownLimitPin:
			
					UnblockTaskWithSemaphore(xWindowFullDownSemaphore);
				 
					break;
    }

}

////////////////////////////////////////////////////////////////////////////

void onPowerBTNSPortInt(void) {

    //Get which pin interrupted
    uint32_t INT_PIN_NUM = GPIOIntStatus(PowerBTNS_GPIO_PORT_BASE, false);

    GPIOIntClear(PowerBTNS_GPIO_PORT_BASE, INT_PIN_NUM); // Clear interrupt flag

    androidINT = false; //not android interrupt
		
    if(!bCentralBtnDebounceReady) //for debouncing central button //If not ready to listen to button change, return.
        return;
    bCentralBtnDebounceReady = false; //If ready, set it to false, until it's set to true again.

    switch(INT_PIN_NUM)
    {

    case CentralBtnDownPin :

        UnblockTaskWithSemaphore(xCentralButtonDownSemaphore);

        break;


    case CentralBtnUpPin :

        UnblockTaskWithSemaphore(xCentralButtonUpSemaphore);

        break;


    case PassengerBtnDownPin :

        UnblockTaskWithSemaphore(xPassengerButtonDownSemaphore);

        break;


    case PassengerBtnUpPin :

        UnblockTaskWithSemaphore(xPassengerButtonUpSemaphore);

        break;

    }

}

void onPortEInt(void) {

    //Get which pin interrupted
    uint32_t INT_PIN_NUM = GPIOIntStatus(GPIO_PORTE_BASE, false);

    GPIOIntClear(GPIO_PORTE_BASE, INT_PIN_NUM); // Clear interrupt flag

    switch(INT_PIN_NUM)
    {

    case EngineStartButton :

        UnblockTaskWithSemaphore(xEngineStartButtonPressedSemaphore);

        break;

    case LockSwitchPin : //In case of switch buttons, we don't need to use Debounce thing

        UnblockTaskWithSemaphore(xPassengLockSemaphore);

        break;
				
				
    case JamPin :
		
				

        break;

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


    //enable limit switches AS PF0 PF4
    //GPIOPinTypeGPIOInput(Limits_GPIO_PORT_BASE, WindowUpLimitPin | WindowDownLimitPin);
    //GPIOPadConfigSet(Limits_GPIO_PORT_BASE, WindowUpLimitPin | WindowDownLimitPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntDisable(Limits_GPIO_PORT_BASE, WindowUpLimitPin | WindowDownLimitPin);
    GPIOIntTypeSet(Limits_GPIO_PORT_BASE, WindowUpLimitPin | WindowDownLimitPin, GPIO_FALLING_EDGE);
    GPIOIntEnable(Limits_GPIO_PORT_BASE, WindowUpLimitPin | WindowDownLimitPin);
    GPIOIntRegister(Limits_GPIO_PORT_BASE, onLimitSwitchesInt);
    ////////////////////////////////////////////////////////////////////


    //enable Central Buttons pins, CentralBTNS_GPIO_PORT_BASE and pin numbers are defined at PORTS.h ////////////////////////////////////
    ROM_SysCtlPeripheralEnable(PowerBTNS_SYSCTL_PERIPH_GPIO);                            //comment this line out if you're trying PF0 and PF4
    GPIOPinTypeGPIOInput(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin); //comment this line out if you're trying PF0 and PF4
    //Central Buttons INTERRUPT INIT //////////////////////////////////
    GPIOIntDisable(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin);
    GPIOIntTypeSet(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin, GPIO_BOTH_EDGES);
    GPIOIntEnable(PowerBTNS_GPIO_PORT_BASE, CentralBtnDownPin | CentralBtnUpPin);
    ////////////////////////////////////////////////////////////////////

    //enable Passenger Buttons pins, PassengerBTNS_GPIO_PORT_BASE and pin numbers are defined at PORTS.h ////////////////////////////////////
    GPIOPinTypeGPIOInput(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin);
    //Passenger Buttons INTERRUPT INIT //////////////////////////////////
    GPIOIntDisable(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin);
    GPIOIntTypeSet(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin, GPIO_BOTH_EDGES);
    GPIOIntEnable(PowerBTNS_GPIO_PORT_BASE, PassengerBtnDownPin | PassengerBtnUpPin);
    ////////////////////////////////////////////////////////////////////
    GPIOIntRegister(PowerBTNS_GPIO_PORT_BASE, onPowerBTNSPortInt);	//Link the method that is going to be called on the interrupt

    bCentralBtnDebounceReady = true;
    

    //PORT E ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    //Motor INIT ////////////////////////////////////////////////////////////////////////////////////////////
    GPIOPinTypeGPIOOutput(Motor_GPIO_PORT_BASE, MotorPinEN | MotorPin1 | MotorPin2);
    GPIOPinWrite(Motor_GPIO_PORT_BASE, MotorPinEN | MotorPin1 | MotorPin2 , 0);

    //Engine INIT ///////////////////////////////////////////////////////////////////////////////////////////
    GPIOPinTypeGPIOInput(EngineStartButton_GPIO_PORT_BASE, EngineStartButton);
    //LINE BELOW ENSURE PULL UP, MUST BE CALLED AFTER GPIOPinTypeGPIOInput, ELSE IT WON'T WORK
    GPIOPadConfigSet(EngineStartButton_GPIO_PORT_BASE, EngineStartButton, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //INT INIT
    GPIOIntDisable(EngineStartButton_GPIO_PORT_BASE, EngineStartButton);
    GPIOIntTypeSet(EngineStartButton_GPIO_PORT_BASE, EngineStartButton, GPIO_BOTH_EDGES );
    GPIOIntEnable(EngineStartButton_GPIO_PORT_BASE, EngineStartButton);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Lock switch and Jam switches INIT
    GPIOPinTypeGPIOInput(Lock_GPIO_PORT_BASE, LockSwitchPin | JamPin);
    //LINE BELOW ENSURES PULL UP, MUST BE CALLED AFTER GPIOPinTypeGPIOInput, ELSE IT WON'T WORK
    GPIOPadConfigSet(Lock_GPIO_PORT_BASE, LockSwitchPin | JamPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntDisable(Lock_GPIO_PORT_BASE, LockSwitchPin | JamPin);
    GPIOIntTypeSet(Lock_GPIO_PORT_BASE, LockSwitchPin , GPIO_BOTH_EDGES);
    GPIOIntTypeSet(Lock_GPIO_PORT_BASE, JamPin , GPIO_FALLING_EDGE);
    GPIOIntEnable(Lock_GPIO_PORT_BASE, LockSwitchPin | JamPin);

    GPIOIntRegister(GPIO_PORTE_BASE, onPortEInt);	//For lock switch + Jam pin + engine start switch
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
