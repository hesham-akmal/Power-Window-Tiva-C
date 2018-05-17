///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////Central Buttons Port and Pins
#define PowerBTNS_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PowerBTNS_GPIO_PORT_BASE GPIO_PORTC_BASE
#define CentralBtnDownPin GPIO_PIN_4
#define CentralBtnUpPin GPIO_PIN_5
#define PassengerBtnDownPin GPIO_PIN_6
#define PassengerBtnUpPin GPIO_PIN_7
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
///////////Engine Start Port
# define Engine_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
# define EngineStartButton_GPIO_PORT_BASE GPIO_PORTC_BASE
# define EngineStartButton GPIO_PIN_7
extern uint32_t EngineTaskInit(void);
extern bool bEngineStarted;
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
enum STATE{Neutral,
					 CentManualOpening, CentManualClosing, 
					 CentAutoOpening, CentAutoClosing, PassManualOpening, PassManualClosing, 
					 PassAutoOpening, PassAutoClosing	};

extern enum STATE State;

enum STATE {Neutral,
            CentManualOpening, CentManualClosing,
            CentAutoOpening, CentAutoClosing
           };

extern enum STATE State;



