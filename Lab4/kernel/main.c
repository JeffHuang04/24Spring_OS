
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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

int strategy;
int delay_time;

PRIVATE void init_tasks(){
	init_screen(tty_table);
	cleanScreen();
	int prior[7] = {1, 1, 1, 1, 1, 1, 1};
	for (int i = 0; i < 7; ++i) {
        proc_table[i].ticks    = prior[i];
        proc_table[i].priority = prior[i];
	}
	k_reenter = 0;
	ticks = 0;
	readers = 0;
	writers = 0;
	strategy = 3;//0：读写公平 1：读者优先 2：写者优先 3:生产者消费者
	delay_time = strategy == 2 ? 1 : 0;
	p_proc_ready = proc_table;
}

void init_semaphore(SEMAPHORE* semaphore,int value){
    semaphore->value = value;
    semaphore->head = 0; 
    semaphore->tail = 0;
}

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
    u8              privilege;
    u8              rpl;
    int             eflags;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
        }
        else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
        }

		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid
		p_proc->sleepTicks = 0;
		p_proc->isBlocked = 0;
		p_proc->isSleep = 0;

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
    this_number[0] = this_number[1] = this_number[2] = this_number[3] = this_number[4] = 0;
	init_tasks();
	init_clock();
    init_keyboard();
	//cleanScreen();
	restart();

	while(1){}
}

PUBLIC void cleanScreen(){
	disp_pos = 0;//记录在屏幕上显示字符的位置
	for(int i = 0; i < SCREEN_SIZE; i++){
		disp_str(" ");
	}
	disp_pos = 0;
}

PRIVATE read_proc(int slices){
    sleep(slices * TIME_SLICE, 0); // 读耗时slices个时间片
}

PRIVATE write_proc(int slices){
    sleep(slices * TIME_SLICE, 0); // 写耗时slices个时间片
}

void produce(int num){
    while (1){
        if(num == 0){
            P(&space);
            //P(&mutex1);
            this_number[num]++;
            sleep(TIME_SLICE,0);
            //V(&mutex1);
            V(&goods1);
        } else {
            P(&space);
            //P(&mutex2);
            this_number[num]++;
            sleep(TIME_SLICE,0);
            //V(&mutex2);
            V(&goods2);
        }
    }
}

void consume(int num){
    while (1){
        if (num == 2){
            P(&goods1);
            //P(&mutex1);
            this_number[num]++;
            sleep(TIME_SLICE,0);
            //V(&mutex1);
            V(&space);
        }else{
            P(&goods2);
            //P(&mutex2);
            this_number[num]++;
            sleep(TIME_SLICE,0);
            //V(&mutex2);
            V(&space);
        }
        
    }
} 

// 读写公平
void read_fair(char proc, int slices){
    P(&queue);
    P(&readerNum_mutex);
    P(&reader_mutex);
    if (++readers == 1)
        P(&rw_mutex); // 有读者正在使用，写者不可抢占
    V(&reader_mutex);
    V(&queue);

    read_proc(slices);
    
    P(&reader_mutex);
    if (--readers == 0)
        V(&rw_mutex); // 没有读者时，可以开始写了
    V(&reader_mutex);
    V(&readerNum_mutex);
}

void write_fair(char proc, int slices){
    P(&queue);
    P(&rw_mutex);
    write_proc(slices);
    V(&rw_mutex);
    V(&queue);
}

// 读者优先 尽量满足并发的读操作，当已经有线程在读数据的时候，其他读线程无需等待，而写线程需要等待所有正在进行的读操作之后才能执行
void read_rf(char proc, int slices){
    P(&reader_mutex);
	if (++readers == 1)
		P(&writer_mutex);//第一个读者申请写者资源-->写者互斥
	V(&reader_mutex);
	
	P(&readerNum_mutex);
	read_proc(slices);
	V(&readerNum_mutex);
	
	P(&reader_mutex);
	if (--readers == 0)
		V(&writer_mutex);//最后一个读者释放写者资源-->写者互斥
	V(&reader_mutex);
}

void write_rf(char proc, int slices){
    P(&rw_mutex);//实现全读者优先
	P(&writer_mutex);
	write_proc(slices);
	V(&writer_mutex);
	V(&rw_mutex);
}

// 写者优先 尽量满足写操作，尽管写操作不能并发，但是可以排队，优先于等待的读线程获得执行权
void read_wf(char proc, int slices){
    P(&readerNum_mutex);
    P(&queue);

    P(&reader_mutex);
    if (++readers == 1)
        P(&rw_mutex);
    V(&reader_mutex);

    V(&queue);

    read_proc(slices);
    
    P(&reader_mutex);
    if (--readers == 0)
        V(&rw_mutex); // 没有读者，可以开始写了
    V(&reader_mutex);

    V(&readerNum_mutex);
}

void write_wf(char proc, int slices){
    P(&writer_mutex);
    if (++writers == 1)
        P(&queue);
    V(&writer_mutex);

    P(&rw_mutex);
    write_proc(slices);
    V(&rw_mutex);

    P(&writer_mutex);
    if (--writers == 0)
        V(&queue);
    V(&writer_mutex);
}

read_f read_funcs[3] = {read_fair, read_rf, read_wf};
write_f write_funcs[3] = {write_fair, write_rf, write_wf};

/*======================================================================*
                               Normal
 *======================================================================*/
void NormalA(){
    sleep(TIME_SLICE, 0);
    char white = '\06';
    char blue = '\03';
    char green = '\02';
    char red = '\04';
    int time = 1;
    while (1){
        if (time <= 20){
            printf("%c%c%c ", white, time / 10 + '0', time % 10 + '0');
            if(strategy != 3){
                for (PROCESS *p = proc_table + NR_TASKS + 1; p < proc_table + NR_TASKS + NR_PROCS; p++){
                    if (strategy == 2 && time == 1 && p < proc_table + 5){
                        p->isSleep = 0;
                        printf("%cX ", red);//强制让第一次休眠的读者进程先改称等待
                    }else if (p->isBlocked){
                        printf("%cX ", red);
                    }else if (p->isSleep == 1){
                        printf("%cZ ", blue);
                    }else if (p->isSleep == 2){
                        p->isSleep = 0;
                        printf("%cZ ", blue);
                    }else {
                        printf("%cO ", green);
                    }
                }
            }else{
                //生产者消费者
                for(int i = 0; i < 5;i++){
                    if(this_number[i]/10 != 0){
                        printf("%c%c%c ",white, this_number[i]/10 + '0',this_number[i]%10 + '0');
                    }else{
                        printf("%c%c  ",white, this_number[i] + '0');
                    }
                }
            }
            printf("\n");
        }
        sleep(TIME_SLICE, 0);
        time++;
    }
}

/*======================================================================*
                               B
 *======================================================================*/
void B(){
    if(strategy != 3){
        sleep(delay_time * TIME_SLICE, 1);
        while (1){
            read_funcs[strategy]('B', 2);
            sleep(RELEX_TIME * TIME_SLICE, 1);
        }
    }else{
        produce(0);
    }
}

/*======================================================================*
                               C
 *======================================================================*/
void C(){
    if(strategy != 3){
        sleep(delay_time * TIME_SLICE, 1);
        while (1){
            read_funcs[strategy]('C', 3);
            sleep(RELEX_TIME * TIME_SLICE, 1);
        }
    }else{
        produce(1);
    }
}

/*======================================================================*
                               D
 *======================================================================*/
void D(){
    if(strategy != 3){
        sleep(delay_time * TIME_SLICE, 1);
        while (1){
            read_funcs[strategy]('D', 3);
            sleep(RELEX_TIME * TIME_SLICE, 1);
        }
    }else{
        consume(2);
    }
}

/*======================================================================*
                               E
 *======================================================================*/
void E(){
    if(strategy != 3){
        while (1){
            write_funcs[strategy]('E', 3);
            sleep(RELEX_TIME * TIME_SLICE, 1);
        }
    }else{
        consume(3);
    }
}

/*======================================================================*
                               F
 *======================================================================*/
void F(){
    if(strategy != 3){
        while (1){
            write_funcs[strategy]('F', 4);
            sleep(RELEX_TIME * TIME_SLICE, 1);
        }
    }else{
        consume(4);
    }
}
