
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void sys_print(char* str){
	disp_str(str);
}

PUBLIC void sys_sleep(int milli_sec){
	p_proc_ready->wake_ticks = get_ticks() + milli_sec / (1000 / HZ);
 	schedule();
}

PUBLIC void sys_P(SEMAPHORE* semaphore){
	disable_irq(CLOCK_IRQ);
	semaphore->value--;
	if (semaphore->value < 0) {
		p_proc_ready->isBlock = 1;
		semaphore->list[semaphore->tail] = p_proc_ready;
		semaphore->tail = (semaphore->tail + 1) % NR_TASKS;
		schedule();
	}
	enable_irq(CLOCK_IRQ);
}

PUBLIC void sys_V(SEMAPHORE* semaphore){
	disable_irq(CLOCK_IRQ);
	semaphore->value++;
	if (semaphore->value <= 0) {
		PROCESS* p=semaphore->list[semaphore->head];
		p->isBlock = 0;
		semaphore->head = (semaphore->head + 1) % NR_TASKS;
	}
	enable_irq(CLOCK_IRQ);
}