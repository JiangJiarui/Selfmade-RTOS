#include <stdint.h>
#include <stdio.h>


#include "task.h"
#include "PortCommon.h"

#define portNVIC_SYSPRI2_REG ( * ( ( volatile uint32_t * ) 0xe000ed20 ) ) //SHPR寄存器

#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )



#define configSYSTICK_CLOCK_HZ 168000000
#define configTICK_RATE_HZ 1000



#define portPendSV_Set() portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;


uint32_t CriticalMark = 0;

void xPortPendSVHandler(void);
void xPortSysTickHandler(void);
void SVC_Handler(void);

static void PortStartFirstTask(void);
/*-------------------------------------------------------------*/

void PortStartScheduler(void)
{
    portNVIC_SYSPRI2_REG |= (((uint32_t)(0xf << 4ul)) << 16ul);     //[23:16]PendSV
    portNVIC_SYSPRI2_REG |= (((uint32_t)(0xf << 4ul)) << 24ul);     //[31:24]SysTick

    /*PortSetupTimerInterrupt*/
    portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
    portNVIC_SYSTICK_CTRL_REG = ( (1ul<<2ul) | (1ul<<1ul) | (1ul<<0ul) );

    PortStartFirstTask();
}

__asm void PortStartFirstTask(void)
{
	PRESERVE8

    ldr r0, =0xE000ED08 //VTOR
    ldr r0, [r0]
    ldr r0, [r0]

		msr msp, r0
	
		cpsie i
		cpsie f
	
		dsb
		isb
	
		SVC 0
		nop
	  nop
}

__asm void xPortSVC_Handler(void)
{
	PRESERVE8
	
	ldr	r3, =pCurrentTCB
	ldr r1, [r3]
	ldr r0, [r1]

	ldmia r0!, {r4-r11, r14}
	msr psp, r0
	isb
	mov r0, #0
	msr	basepri, r0
	bx r14
}

__asm void xPortPendSVHandler( void )
{
	extern pCurrentTCB
	extern vTaskSwitchContext

	PRESERVE8
	
	ldr r3, =pCurrentTCB
	ldr r2, [r3]

	mrs r0, psp
	
	stmdb r0!, {r4-r11, r14}
	msr psp, r0
	str r0, [r2]
	
	stmdb sp!, {r3}
	
	mov r0, #SysCall_Interrupt_Priority
	msr basepri, r0
	isb
	dsb
	bl vTaskSwitchContext
	mov r0, #0
	msr basepri, r0
	isb
	dsb
	
	ldmia sp!, {r3}
	ldr r1, [r3]
	ldr r0, [r1]

	ldmia r0!, {r4-r11, r14}
	msr psp, r0
	
	bx r14
	
}
 


void xPortSysTickHandler(void)
{
	vPortEnterCritical();
	
	if(TaskTimerIncrement() != pdFALSE)
	{
		portPendSV_Set();
	}
	vPortExitCritical();
}



