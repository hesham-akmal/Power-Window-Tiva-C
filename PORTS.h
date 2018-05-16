///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////Central Buttons Port and Pins
#define CentralBTNS_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define CentralBTNS_GPIO_PORT_BASE GPIO_PORTC_BASE
#define CentralBtnDownPin GPIO_PIN_4
#define CentralBtnUpPin GPIO_PIN_5
//////////////////////
/*
//trying with PF0 and PF4 for Central Buttons
# define CentralBTNS_GPIO_PORT_BASE GPIO_PORTF_BASE
# define CentralBtnDownPin GPIO_PIN_0
# define CentralBtnUpPin GPIO_PIN_4
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////Motor Port and Pins
# define Motor_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOE
# define Motor_GPIO_PORT_BASE GPIO_PORTE_BASE
# define MotorPinEN GPIO_PIN_1
# define MotorPin1 GPIO_PIN_2
# define MotorPin2 GPIO_PIN_3
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t ListenTaskInit(void);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//From buttons.c //////////////////////
extern uint8_t INT_PIN_NUM;
extern bool bCentralBtnDebounceReady;
///////////////////////////////////////
//From android_listen.c ///////////////
extern bool androidINT;
///////////////////////////////////////



/*
StateType FSM[5]={
 {false,false, false, false, false, {goN,waitN,goN,waitN}},  //next states should be firstDelay Down/up
 {0x22, 500,{goE,goE,goE,goE}},
 {0x0C,3000,{goE,goE,waitE,waitE}},
 {0x14, 500,{goN,goN,goN,goN}},
 {true, true, false, true, false, {goN,goN,goN,goN}}};*/

