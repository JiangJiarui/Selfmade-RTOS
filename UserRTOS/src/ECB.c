

#include <stdio.h>

#include "ECB.h"
#include "task.h"

extern List_t TaskReadyList[MAX_Priority];
extern TCB_t * volatile pCurrentTCB;
extern volatile uint32_t xYieldPending;
extern volatile uint32_t uxReadyPriorities;
extern volatile uint32_t TickCount;


static uint32_t RemovefromEventList(List_t * EventList);

/*-----------------------------------------------*/


static uint32_t RemovefromEventList(List_t * EventList)
{
	TCB_t * pxTCB = NULL;
	uint32_t xReturn;

	pxTCB = (TCB_t *)(EventList->ListEnd.pNextItem->pOwner);
	(void) TaskListItemRemove(&(pxTCB->xEventItem));
	
	(void) TaskListItemRemove(&(pxTCB->xStateItem));
	

	(void)vTaskAddToReadyList(&TaskReadyList[pxTCB->Priority], pxTCB);

	if(pxTCB->Priority > pCurrentTCB->Priority)
	{
		xReturn = pdTRUE;
		xYieldPending = pdTRUE;
	}
	else
	{
		xReturn = pdFALSE;
	}
	
	return xReturn;
}

	
SemHandle SemCreate(uint32_t SemLength)
{
	ECB_t * pxECB = NULL;
	for(int i = 0; i < MAX_Sem; i++)
	{
		if(SemPool[i].SemDefined == 0)
		{		
			pxECB = &SemPool[i];
			break;
		}
	}
	
	pxECB->SemDefined = 1;
	pxECB->event = Semaphore;
	pxECB->Length = SemLength;
	
	vListInitialise(&(pxECB->WaitTo_Receive_List));
	vListInitialise(&(pxECB->WaitTo_Send_List));

	return pxECB;
}

uint32_t SemGive(SemHandle Sem, uint32_t BlockTime)
{
	ECB_t * const pxECB = Sem;
	uint32_t EntryTime;
	uint32_t EntryTimeisSet = 0;
	
	for(;;)
	{
		
		EnterCritical();
		{
		
				if((pxECB->ReNum) < (pxECB->Length))
				{
					(pxECB->ReNum)++;
					
					if(pxECB->WaitTo_Receive_List.ListItemNum != 0)
					{
						if(RemovefromEventList(&(pxECB->WaitTo_Receive_List)) != pdFALSE)
						{
							portYIELD();
						}
					}
					
					ExitCritical();
					return 1;
				}
				else
				{
					if(BlockTime == 0)
					{
						ExitCritical();
						return 0;
					}
					else
					{
						if(EntryTimeisSet == 0)
						{
							EntryTime = TickCount;
							EntryTimeisSet = 1;
						}
					}
					
				}
		
		}
		
		ExitCritical();
		
		
		EnterCritical();
		vTaskSuspendAll();
		{
			if((TickCount - EntryTime) < BlockTime)
			{
				if(pxECB->WaitTo_Send_List.ListItemNum == 0)
				{
					vListItemInsert(&(pxECB->WaitTo_Send_List), &(pCurrentTCB->xEventItem));
					vTaskAddToDelayList(BlockTime);
				}
				xYieldPending = pdTRUE;
			}	
			else
			{
				return 0;
			}
		}
		vTaskResumeAll();
		ExitCritical();
		
	}
}

uint32_t SemTake(SemHandle Sem, uint32_t BlockTime)
{
	ECB_t * const pxECB = Sem;
	uint32_t EntryTime;
	uint32_t EntryTimeisSet = 0;
	
	
	for(;;)
	{
		EnterCritical();
		{
			if(pxECB->ReNum > 0)
			{
				(pxECB->ReNum)--;
				if(pxECB->WaitTo_Send_List.ListItemNum != 0)
				{
					if(RemovefromEventList(&(pxECB->WaitTo_Send_List)) != pdFALSE)
					{
						portYIELD();
					}
				}
				ExitCritical();
				return 1;
			}
			else
			{
				if(BlockTime == 0)
				{
					ExitCritical();
					return 0;
				}
				else if(EntryTimeisSet == 0)
				{
					EntryTime = TickCount;
					EntryTimeisSet = 1;
				}
			}
		}
		ExitCritical();
	
		EnterCritical();
		vTaskSuspendAll();
		{
			if((TickCount - EntryTime) < BlockTime)
			{
				if(pxECB->WaitTo_Receive_List.ListItemNum == 0)
				{
					vListItemInsert(&(pxECB->WaitTo_Receive_List), &(pCurrentTCB->xEventItem));
					vTaskAddToDelayList(BlockTime);
				}
				xYieldPending = pdTRUE;
			}
			else
			{
				return 0;
			}
		}
		vTaskResumeAll();
		ExitCritical();
	
	}
	
}




