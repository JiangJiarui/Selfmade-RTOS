#ifndef ECB_H
#define ECB_H

#include <stdint.h>
#include "list.h"

typedef enum EventType{
	Semaphore, MailBox
}EventType_t;



typedef struct ECB
{
	uint32_t SemDefined;
	EventType_t event;		//事件类型
	List_t WaitTo_Send_List;
	List_t WaitTo_Receive_List;
	uint32_t ReNum;
	uint32_t Length;
}ECB_t;

typedef ECB_t* SemHandle;



#define MAX_Sem 5

static ECB_t SemPool[MAX_Sem];



SemHandle SemCreate(uint32_t SemLength);
uint32_t SemGive(SemHandle Sem, uint32_t BlockTime);
uint32_t SemTake(SemHandle Sem, uint32_t BlockTime);


#define Get_Sem_State(SemHandle) (SemHandle->ReNum);




#endif //ECB_H
