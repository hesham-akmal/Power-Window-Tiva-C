

#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle xCentralButtonUpSemaphore;
extern xSemaphoreHandle xCentralButtonDownSemaphore;
					 
extern xSemaphoreHandle xPassengerButtonUpSemaphore;
extern xSemaphoreHandle xPassengerButtonDownSemaphore;

uint32_t statesTasksInit(void);
void LimitSwitchUp(void);
void LimitSwitchDown(void);
void LockSwitch(void);
