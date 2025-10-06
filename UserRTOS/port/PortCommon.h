#ifndef PortCommon_H
#define PortCommon_H


#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )

#define SysCall_Interrupt_Priority	(5 << 4)
#define portMAX_DELAY 0xffffffffUL
#define MAX_Priority 5


#define xPortSVC_Handler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler



__forceinline void vPortEnterCritical( void )
{
	__asm
	{
		msr basepri, #SysCall_Interrupt_Priority
		dsb
		isb
	}
}

__forceinline void vPortExitCritical( void )
{
	__asm
	{
		msr basepri, #0
		dsb
		isb
	}
}

#define EnterCritical() vPortEnterCritical();
#define ExitCritical()	vPortExitCritical();

#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - ( uint32_t ) __clz( ( uxReadyPriorities ) ) )

#define portYIELD()																\
{																									\
	portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;	\
}																									\

void PortStartScheduler(void);



#endif	//PortCommon_H
