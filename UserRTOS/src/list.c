#include <stdlib.h>

#include "list.h"


void vListInitialise(List_t * NewList)
{
	NewList->ListItemNum = 0ul;
	
	NewList->ListEnd.ItemVlaue = portMAX_DELAY;
	NewList->ListEnd.pNextItem = &(NewList->ListEnd);
	NewList->ListEnd.pPreviousItem = &(NewList->ListEnd);
	NewList->ListEnd.pOwner = NULL;
	NewList->ListEnd.pContainer = NULL;
	
	NewList->pIndex = &(NewList->ListEnd);
}

void vListItem_Init(ListItem_t * newItem)
{
	newItem->pContainer = NULL;
}

void vListItemInsert(List_t * WantedList, ListItem_t * AddedItem)
{
	ListItem_t * pIterator = NULL;
	

	for(pIterator = &(WantedList->ListEnd); pIterator->pNextItem->ItemVlaue <= AddedItem->ItemVlaue; pIterator = pIterator->pNextItem)
	{
		;
	}
	
	AddedItem->pNextItem = pIterator->pNextItem;
	AddedItem->pPreviousItem = pIterator;
	AddedItem->pNextItem->pPreviousItem = AddedItem;
	AddedItem->pPreviousItem->pNextItem = AddedItem;
	AddedItem->pContainer = WantedList;
	
	(WantedList->ListItemNum)++;
}

void vListItemInsert_End(List_t * WantedList, ListItem_t * AddedItem)
{
	ListItem_t * pxIndex = WantedList->pIndex;
	
	
	AddedItem->pContainer = WantedList;
	AddedItem->pNextItem = pxIndex;
	
	
	AddedItem->pPreviousItem = pxIndex->pPreviousItem;
	AddedItem->pPreviousItem->pNextItem = AddedItem;
	
	pxIndex->pPreviousItem = AddedItem;
	
	(WantedList->ListItemNum)++;
}


void vListRemove(ListItem_t * RemovedItem)
{
	List_t * pxContainer = NULL;
	
	pxContainer = RemovedItem->pContainer;
	
	RemovedItem->pPreviousItem->pNextItem = RemovedItem->pNextItem;
	RemovedItem->pNextItem->pPreviousItem = RemovedItem->pPreviousItem;

	if(pxContainer->pIndex == RemovedItem)
	{
		pxContainer->pIndex = RemovedItem->pPreviousItem;
	}
	RemovedItem->pNextItem = NULL;
	RemovedItem->pPreviousItem = NULL;
	
	(pxContainer->ListItemNum)--;
	RemovedItem->pContainer = NULL;
}
