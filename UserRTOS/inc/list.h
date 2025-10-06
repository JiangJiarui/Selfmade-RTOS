#ifndef LIST_H
#define LIST_H

#include <stdint.h>

#include "PortCommon.h"

typedef struct ListItem
{
	uint32_t ItemVlaue;
	struct ListItem * pNextItem;
	struct ListItem * pPreviousItem;
	void * pContainer;
	void * pOwner;
}ListItem_t;


typedef struct List
{
	uint32_t ListItemNum;
	struct ListItem * pIndex;
	struct ListItem ListEnd;
}List_t;
/*----------------------------------------------------------*/

void vListInitialise(List_t * NewList);
void vListItem_Init(ListItem_t * newItem);
void vListItemInsert(List_t * WantedList, ListItem_t * AddedItem);
void vListItemInsert_End(List_t * WantedList, ListItem_t * AddedItem);
void vListRemove(ListItem_t * RemovedItem);





#endif //LIST_H
