
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule(){
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table ; p < proc_table + NR_TASKS + NR_PROCS; p++) {
			if(p->sleepTicks > 0 || p->isBlocked){
				continue;
			}
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for(p=proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
				if (p->ticks > 0){
					continue;
				}
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

PUBLIC void sys_output_str(char* str, int len){
	CONSOLE* p_con = console_table;
	for (int i = 0;i < len; i++){
		out_char(p_con, str[i]);
	}
}

PUBLIC void sys_sleep(int milli_sec,int type){
	p_proc_ready->sleepTicks = milli_sec / 1000 * HZ * 10;
    if(type == 1){
        p_proc_ready->isSleep = 1;//休眠状态
    } else{
        p_proc_ready->isSleep = 0;//运行状态
    }
    schedule();
}

PUBLIC void sys_P(SEMAPHORE* semaphore){
	disable_int();//关中断
	semaphore->value--;
	if(semaphore->value < 0){
		p_proc_ready->isBlocked = 1;
		semaphore->wait_list[semaphore->tail] = p_proc_ready;
		semaphore->tail = (semaphore->tail + 1) % NR_PROCS;
		schedule();
	}
	enable_int();
}

PUBLIC void sys_V(SEMAPHORE* semaphore){
	disable_int();//关中断
	semaphore->value++;
	if (semaphore->value <= 0){
		semaphore->wait_list[semaphore->head]->isBlocked = 0; // 唤醒最先进入队列的进程
		semaphore->head = (semaphore->head + 1) % NR_PROCS;
	}
	enable_int();
}