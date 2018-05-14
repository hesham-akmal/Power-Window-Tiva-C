#include <stdbool.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#include "LCD.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "drivers/buttons.h"
#include "FreeRTOS.h"

//LCD Port and Pins
///ctrl
#define LCD_ctrl_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOA
#define LCD_ctrl_GPIO_PORT_BASE GPIO_PORTA_BASE
#define LCD_ctrl_DATA_BASE GPIO_PORTA_DATA_R
#define LCD_ctrl_pin1 GPIO_PIN_5
#define LCD_ctrl_pin2 GPIO_PIN_6
#define LCD_ctrl_pin3 GPIO_PIN_7
///data
#define LCD_data_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOB
#define LCD_data_GPIO_PORT_BASE GPIO_PORTB_BASE
#define LCD_data_DATA_BASE GPIO_PORTB_DATA_R
/////////////////////////////////////////

void Delay_ms(int ms) {
    ROM_SysCtlDelay( (ROM_SysCtlClockGet()/(3*1000))*ms );
}

//This Function is used to send commands to the LCD
void LCD_command(unsigned char command)
{
    LCD_ctrl_DATA_BASE = 0;
    LCD_data_DATA_BASE = command;

    LCD_ctrl_DATA_BASE = 0x80;
    Delay_ms(1);
    LCD_ctrl_DATA_BASE = 0;

    Delay_ms(2);         // command 1 and 2 needs up to 1.64ms
}

void LCD_data(unsigned char data)
{
    LCD_ctrl_DATA_BASE = 0x20;    // RS(0x20) = 1 for data, R/W = 0
    LCD_data_DATA_BASE = data;

    LCD_ctrl_DATA_BASE |= 0x80; 	// pulse E
    Delay_ms(1);
    LCD_ctrl_DATA_BASE = 0;

    Delay_ms(1);
}

void LCD_INIT(void) {
    ////////////////////////////////////////////////////////////////////////
    //Enable Clocks for LCD ctrl and LCD data
    ROM_SysCtlPeripheralEnable(LCD_ctrl_SYSCTL_PERIPH_GPIO);
    ROM_SysCtlPeripheralEnable(LCD_data_SYSCTL_PERIPH_GPIO);
    //Output init for LCD_ctrl
    GPIOPinTypeGPIOOutput(LCD_ctrl_GPIO_PORT_BASE, LCD_ctrl_pin1);
    GPIOPinTypeGPIOOutput(LCD_ctrl_GPIO_PORT_BASE, LCD_ctrl_pin2);
    GPIOPinTypeGPIOOutput(LCD_ctrl_GPIO_PORT_BASE, LCD_ctrl_pin3);
    //Output init for LCD_data
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOOutput(LCD_data_GPIO_PORT_BASE, GPIO_PIN_7);
    ////////////////////////////////////////////////////////////////////////

    // Wake Up LCD Sequence
    Delay_ms(20);            /* initialization sequence */// Wait >15 ms after power is applied
    LCD_command(0x30);		// command 0x30 = Wake up
    Delay_ms(5);				  // must wait 5ms, busy flag not available

    //Prepare LCD Operation and Function
    LCD_command(0x38);      /* set 8-bit data, 2-line, 5x7 font */
    LCD_command(0x01);      /* clear screen, move cursor to home */
    LCD_command(0x0C);      /* turn on display, cursor off */
}

void LCD_print_string(char * string) {
    LCD_clear();
    uint16_t i = 0;
    for(i = 0; string[i] != '\0'; i++)
        LCD_data( string[i] );
}


void LCD_clear(void) {
    LCD_command(0x01);
}
