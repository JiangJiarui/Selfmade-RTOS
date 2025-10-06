#include <stdlib.h>


#include "task.h"
#include "portcommon.h"

#define portINITIAL_XPSR ( 0x01000000 )
#define portINITIAL_EXEC_RETURN		( 0xfffffffd )
#define portSTART_ADDRESS_MASK ((uint32_t)(0xfffffffeUL))



TCB_t * volatile pCurrentTCB = NULL;

volatile uint32_t SchedulerSuspended = pdFALSE;
volatile uint32_t TickCount = 0;
volatile uint32_t xNextTaskUnblockTime = portMAX_DELAY;
static volatile uint32_t uxPendedTicks = 0U;
volatile uint32_t xYieldPending = pdFALSE;
static volatile uint32_t uxSchedulerSuspended	= pdFALSE;
static uint32_t CurrentTaskNum = 0;
volatile uint32_t uxReadyPriorities = 0ul;
void vTaskAddToReadyList(List_t * WantedList, TCB_t * AddedTCB);


List_t TaskReadyList[MAX_Priority];
List_t TaskDelayList;
List_t TaskSuspendedList;
/*--------------------------------------------------------------*/

static volatile uint32_t * TaskStackInitialise(Function_t FunctionName, volatile uint32_t *pxTopOfStack);
static void TaskListInit(void);



/*--------------------------------------------------------------*/
void vTaskAddToDelayList(const uint32_t TickToDelay)
{
	uint32_t TimeToWake;
	const uint32_t CurrentTickCount = TickCount;
	
	if(TaskListItemRemove(&(pCurrentTCB->xStateItem)) == 0)
	{
		uxReadyPriorities &= ~(1 << (pCurrentTCB->Priority));
	}
	
	TimeToWake = CurrentTickCount + TickToDelay;
	
	pCurrentTCB->xStateItem.ItemVlaue = TimeToWake;
	
	vListItemInsert(&TaskDelayList, &(pCurrentTCB->xStateItem));
	if(TimeToWake < xNextTaskUnblockTime)
	{
		xNextTaskUnblockTime = TimeToWake;
	}
}


void vTaskDelay(const uint32_t TickToDelay)
{
//	TCB_t * const pxTCB = pCurrentTCB;
	vTaskSuspendAll();
	{
		vTaskAddToDelayList(TickToDelay);
	}
	vTaskResumeAll();
	
	portYIELD();
}


uint32_t TaskListItemRemove(ListItem_t * RemovedItem)
{
	List_t * pxList = RemovedItem->pContainer;
	uint32_t rItemValue;
	vListRemove(RemovedItem);
	
	rItemValue = pxList->ListItemNum;
	return rItemValue;
}	

void TaskSuspend(TaskHandle * SuspendedTask)
{
	TCB_t * pxTCB;
	TCB_t * pDelayTCB = NULL;
	EnterCritical();
	{
		pxTCB = (TCB_t *)SuspendedTask;
		if(pxTCB == NULL)
		{
			pxTCB = pCurrentTCB;
		}
		
		if(TaskListItemRemove(&(pxTCB->xStateItem))==0)
		{
			uxReadyPriorities &= ~(1<<(pxTCB->Priority));
		}
		
		if(pxTCB->xEventItem.pContainer != NULL)
		{
			TaskListItemRemove(&(pxTCB->xEventItem));
		}
		
		vListItemInsert_End(&TaskSuspendedList, &(pxTCB->xStateItem));
	}
	ExitCritical();
	

	EnterCritical();
	{
		if(TaskDelayList.ListItemNum == 0)
		{
			xNextTaskUnblockTime = portMAX_DELAY;
		}
		else
		{
			pDelayTCB = TaskDelayList.ListEnd.pNextItem->pOwner;
			xNextTaskUnblockTime = pDelayTCB->xStateItem.ItemVlaue;
		}
	}
	ExitCritical();
	
	if(pxTCB == pCurrentTCB)
	{
		portYIELD();
	}
	
	
}


void TaskResume(TaskHandle * ResumedTask)
{
	TCB_t * const pxTCB = (TCB_t *)ResumedTask;
	
	if((pxTCB != NULL)&&(pxTCB != pCurrentTCB))
	{
		EnterCritical();
		{
			if(pxTCB->xStateItem.pContainer == &TaskSuspendedList)
			{
				TaskListItemRemove(&(pxTCB->xStateItem));
				vTaskAddToReadyList(&TaskReadyList[pxTCB->Priority], pxTCB);
				
				if(pxTCB->Priority >= pCurrentTCB->Priority)
				{
					portYIELD();
				}
			}
		}
		ExitCritical();
	}
}



TaskHandle TaskCreate(Function_t FunctionName, uint32_t TaskPriority, const uint16_t StackSize, StackType * const StackPointer,  TCB_t ** ppTaskTCB)
{
	TCB_t * pxNewTCB = NULL;
	uint32_t * pxTaskStackPointer = NULL;
	
	
		pxTaskStackPointer = StackPointer;

    pxNewTCB = (TCB_t *)(StackPointer + StackSize);

		pxNewTCB->pStack = pxTaskStackPointer;
	
		*ppTaskTCB = pxNewTCB;
		(*ppTaskTCB)->pStack = pxNewTCB->pStack;
	
    (*ppTaskTCB) -> Priority = TaskPriority;
		
		(*ppTaskTCB)->xStateItem.pOwner = (*ppTaskTCB);
		
		TaskListInit();
		
		if(pCurrentTCB == NULL)
		{
			pCurrentTCB = (*ppTaskTCB);
		}	
		else
		{
			if(pCurrentTCB->Priority <= (*ppTaskTCB)->Priority)
			{
				pCurrentTCB = (*ppTaskTCB);
			}
		}
		
		vListItem_Init(&((*ppTaskTCB)->xStateItem));
		vListItem_Init(&((*ppTaskTCB)->xEventItem));
		
		
		
		
		(*ppTaskTCB)->xStateItem.pOwner = (*ppTaskTCB);
		(*ppTaskTCB)->xEventItem.pOwner = (*ppTaskTCB);
		
		(*ppTaskTCB)->xEventItem.ItemVlaue = (MAX_Priority - TaskPriority);

		
//		vListItemInsert_End(&TaskReadyList[TaskPriority], &((*ppTaskTCB)->xStateItem));
		vTaskAddToReadyList(&TaskReadyList[TaskPriority], (*ppTaskTCB));
    
		(*ppTaskTCB)->pTopOfStack = (*ppTaskTCB) ->pStack + ((uint32_t)StackSize-(uint32_t)1);
		
    (*ppTaskTCB)->pTopOfStack = TaskStackInitialise(FunctionName, (*ppTaskTCB)->pTopOfStack);
		
    return (*ppTaskTCB);
}

void vTaskAddToReadyList(List_t * WantedList, TCB_t * AddedTCB)
{
		vListItemInsert_End(WantedList, &(AddedTCB->xStateItem));
		
		uxReadyPriorities |= (1 << (AddedTCB->Priority));
}

static volatile uint32_t * TaskStackInitialise(Function_t FunctionName, volatile uint32_t *pxTopOfStack)
{
	pxTopOfStack--;

	*pxTopOfStack = portINITIAL_XPSR;	/* xPSR */
	pxTopOfStack--;
	*pxTopOfStack = ( ( uint32_t ) FunctionName ) & portSTART_ADDRESS_MASK;	/* PC */
	pxTopOfStack--;
	*pxTopOfStack = 0;	/* LR */

	/* Save code space by skipping register initialisation. */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1. */
	*pxTopOfStack = 0;	/* R0 */

	/* A save method is being used that requires each task to maintain its
	own exec return value. */
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_EXEC_RETURN;

	pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */
	
	return pxTopOfStack;
}

static void TaskListInit(void)
{
	uint32_t xpriority;
	
	CurrentTaskNum++;
	if(CurrentTaskNum == 1)
	{
		for(xpriority = 0; xpriority <= MAX_Priority; xpriority++)
		{
			vListInitialise(TaskReadyList + xpriority);
		}
		
		vListInitialise(&TaskDelayList);
		vListInitialise(&TaskSuspendedList);
		
	}
}

void vTaskSwitchContext( void )
{
	static TCB_t * xCurrentTCB = NULL;
	uint32_t uxTopPriority;
//	static List_t * xTopList = NULL;
	if(SchedulerSuspended != pdFALSE)
	{
		xYieldPending = pdTRUE;
	}
	else
	{
		xYieldPending = pdFALSE;
	}
	
	portGET_HIGHEST_PRIORITY(uxTopPriority, uxReadyPriorities);
	
	TaskReadyList[uxTopPriority].pIndex = TaskReadyList[uxTopPriority].pIndex->pNextItem;
	
	if(TaskReadyList[uxTopPriority].pIndex == &(TaskReadyList[uxTopPriority].ListEnd))
	{
		TaskReadyList[uxTopPriority].pIndex = TaskReadyList[uxTopPriority].pIndex->pNextItem;
	}
	
	xCurrentTCB = TaskReadyList[uxTopPriority].pIndex->pOwner;
	pCurrentTCB = xCurrentTCB;
}

uint32_t TaskTimerIncrement(void)
{
	TCB_t * pxTCB = NULL;
	uint32_t DelayItemValue = 0;
	uint32_t SwitchRequired = pdFALSE;
	if(SchedulerSuspended == pdFALSE)
	{
		uint32_t const CurrentTick = TickCount + 1;
		TickCount = CurrentTick;
		
		
		if(CurrentTick >= xNextTaskUnblockTime)
		{
			for(;;)
			{
				if(TaskDelayList.ListItemNum == 0)
				{
					xNextTaskUnblockTime = portMAX_DELAY;
					break;
				}
				else
				{
					pxTCB = TaskDelayList.ListEnd.pNextItem->pOwner;
					DelayItemValue = TaskDelayList.ListEnd.pNextItem->ItemVlaue;
					if(CurrentTick < DelayItemValue)
					{
						xNextTaskUnblockTime = DelayItemValue;
						break;
					}
					else
					{
						vListRemove(&(pxTCB->xStateItem));
//						vListItemInsert(&TaskReadyList[pxTCB->Priority], &(pxTCB->xStateItem));
						vTaskAddToReadyList(&TaskReadyList[pxTCB->Priority], pxTCB);
						
						if(pxTCB->xEventItem.pContainer != NULL)
						{
							vListRemove(&(pxTCB->xEventItem));
						}
					}
					if(pxTCB->Priority >= pCurrentTCB->Priority)
					{
						SwitchRequired = pdTRUE;
					}
				}
			}
		}
		
		if(TaskReadyList[pCurrentTCB->Priority].ListItemNum > 1)
		{
			SwitchRequired = pdTRUE;
		}	
	}
	
//	++uxPendedTicks;
	if(xYieldPending != pdFALSE)
	{
		SwitchRequired = pdTRUE;
	}

	return SwitchRequired;
}

void vTaskSuspendAll(void)
{
	uxSchedulerSuspended = pdTRUE;
}

void vTaskResumeAll( void )
{
	EnterCritical();
	{
		SchedulerSuspended = pdFALSE;
	}
	ExitCritical();
}
