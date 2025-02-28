
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"


/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;//全局时钟计数器增加
	p_proc_ready->ticks--;//当前进程时间片减少
        for (PROCESS* p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++){
                if (p->sleepTicks > 0){
                        p->sleepTicks--;
                        if (p->sleepTicks == 0 && p->isSleep == 1){
                                p->isSleep = 2;
                        }
                }
        }
	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();

}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler);    /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                        /* 让8259A可以接收时钟中断 */
}


