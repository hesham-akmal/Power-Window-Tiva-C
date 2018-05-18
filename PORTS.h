///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////Central Buttons Port and Pins
#define PowerBTNS_SYSCTL_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PowerBTNS_GPIO_PORT_BASE GPIO_PORTC_BASE
#define CentralBtnDownPin GPIO_PIN_4
#define CentralBtnUpPin GPIO_PIN_5
#define PassengerBtnDownPin GPIO_PIN_6
#define PassengerBtnUpPin GPIO_PIN_7



//limit switches
//////////////////////
#define Limits_GPIO_PORT_BASE GPIO_PORTF_BASE
#define WindowUpLimitPin GPIO_PIN_0
#define WindowDownLimitPin GPIO_PIN_4



//////////////////////
/*
//trying with PF0 and PF4 for Central Buttons
# define CentralBTNS_GPIO_PORT_BASE GPIO_PORTF_BASE
# define CentralBtnDownPin GPIO_PIN_0
# define CentralBtnUpPin GPIO_PIN_4
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////Motor Port and Pins
# define Motor_GPIO_PORT_BASE GPIO_PORTE_BASE
# define MotorPinEN GPIO_PIN_1
# define MotorPin1 GPIO_PIN_2
# define MotorPin2 GPIO_PIN_3
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//lock  and Jam
# define Lock_GPIO_PORT_BASE GPIO_PORTE_BASE
# define LockSwitchPin GPIO_PIN_4
# define JamPin GPIO_PIN_5

///////////Engine Start Port
# define EngineStartButton_GPIO_PORT_BASE GPIO_PORTE_BASE
# define EngineStartButton GPIO_PIN_0
extern uint32_t EngineTaskInit(void);
extern bool bEngineStarted;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//From buttons.c //////////////////////
extern bool bCentralBtnDebounceReady;
extern bool passLocked;
///////////////////////////////////////

//From android_listen.c ///////////////
extern bool androidINT;
uint32_t ListenTaskInit(void);
///////////////////////////////////////

extern bool passLocked;
void CheckLockSwitch(void);

enum STATE {Neutral,
            CentManualOpening, CentManualClosing,
            PassManualOpening, PassManualClosing,
            CentAutoOpening, CentAutoClosing,
            PassAutoOpening, PassAutoClosing,
            FullyClosed, FullyOpened
           };

extern enum STATE State;



