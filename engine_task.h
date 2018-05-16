
//*****************************************************************************

#ifndef __ENGINE_TASK_H__
#define __ENGINE_TASK_H__

#include "FreeRTOS.h"
#include "semphr.h"
//*****************************************************************************
//
// Prototypes for the switch task.
//
//*****************************************************************************

extern xSemaphoreHandle xEngineStartButtonPressedSemaphore;

extern uint32_t EngineTaskInit(void);

#endif // __SWITCH_TASK_H__
