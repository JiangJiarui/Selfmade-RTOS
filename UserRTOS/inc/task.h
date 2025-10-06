#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stddef.h>

#include "list.h"


#define pdFALSE 0
#define pdTRUE	1

typedef struct TCB
{
    volatile uint32_t *pTopOfStack;
    uint32_t Priority;
    volatile uint32_t *pStack;
		ListItem_t xStateItem;
		ListItem_t xEventItem;
}TCB_t;
/*----------------------------------------------------*/


typedef void (*Function_t)(void*);
typedef void* TaskHandle;
typedef uint32_t StackType;

/*----------------------------------------------------*/
TaskHandle TaskCreate(Function_t FunctionName, uint32_t TaskPriority,const uint16_t StackSize, StackType * const,  TCB_t ** ppTaskTCB);
void vTaskSwitchContext( void );
uint32_t TaskTimerIncrement(void);
void TaskSuspend(TaskHandle * SuspendedTask);
void TaskResume(TaskHandle * ResumedTask);
void vTaskSuspendAll(void);
void vTaskResumeAll( void );
void vTaskDelay(const uint32_t TickToDelay);
uint32_t TaskListItemRemove(ListItem_t * RemovedItem);
void vTaskAddToDelayList(const uint32_t TickToDelay);
void vTaskAddToReadyList(List_t * WantedList, TCB_t * AddedTCB);



/*----------------------------------------------------*/



#endif //TASK_H
