
#include "task.h"
#include "portcommon.h"
#include "ECB.h"

#include "bsp_usart.h"


StackType IdleTask_Heap[512] = {0};
StackType Task1_Heap[512] = {0};
StackType Task2_Heap[512] = {0};

TCB_t * IdleTask_TCB;
TCB_t * Task1_TCB;
TCB_t * Task2_TCB;


TaskHandle IdleTask_Handle = NULL;
TaskHandle Task1_Handle = NULL;
TaskHandle Task2_Handle = NULL;

SemHandle Sem1 = NULL;

int flag1 = 0;
int flag2 = 0;

// IdelTask
void IdleTask(void * parameter)
{
	while(1)
		;
}


void Task1(void * parameter)
{
	uint32_t xReturn = 0;
	while(1)
	{
		xReturn = SemTake(Sem1, 0xffff);
		if(xReturn != 0)
		{
			flag1 = flag1 ? 0 : 1;
			vTaskDelay(5);
			xReturn = 0;
			(void)SemGive(Sem1, 0xffff);
		}
	}
}

void Task2(void * parameter)
{
	uint32_t xReturn = 0;
	while(1)
	{
		xReturn = SemTake(Sem1, 0xffff);
		if(xReturn != 0)
		{			
			flag2 = flag2 ? 0 : 1;
			xReturn = 0;
			(void)SemGive(Sem1, 0xffff);
			vTaskDelay(5);
		}
	
	}
}

int main(void)
{
//	USART_Config();
	
	Sem1 = SemCreate(1);
	(void)SemGive(Sem1, 0);
	
	IdleTask_Handle = TaskCreate(IdleTask, 0, 64, IdleTask_Heap, &IdleTask_TCB);
	Task1_Handle = TaskCreate(Task1, 1, 64, Task1_Heap, &Task1_TCB);
	Task2_Handle = TaskCreate(Task2, 2, 64, Task2_Heap, &Task2_TCB);

	PortStartScheduler();
}



