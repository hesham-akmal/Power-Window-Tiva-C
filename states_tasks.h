

#include "FreeRTOS.h"
#include "semphr.h"

extern xSemaphoreHandle xCentralButtonUpSemaphore;
extern xSemaphoreHandle xCentralButtonDownSemaphore;

uint32_t statesTasksInit(void);

