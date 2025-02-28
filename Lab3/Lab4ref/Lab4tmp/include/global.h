
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		    ticks;
EXTERN  int		    readers;
EXTERN  int		    writers;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];

EXTERN	u32		k_reenter;

EXTERN	TSS		tss;
EXTERN	PROCESS*	p_proc_ready;

EXTERN	int		nr_current_console;

extern	PROCESS		    proc_table[];
extern	char		    task_stack[];
extern  TASK            task_table[];
extern  TASK            user_proc_table[];
extern	irq_handler	    irq_table[];
extern	TTY		        tty_table[];
extern  CONSOLE         console_table[];
extern int mode;
//读写问题
extern  SEMAPHORE   rw_mutex;
extern  SEMAPHORE   writer_mutex;
extern  SEMAPHORE   reader_mutex;
extern  SEMAPHORE   queue;
extern  SEMAPHORE   readerNum_mutex;

EXTERN int this_number[5];// = {0,0,0,0,0};
//生产者消费者问题
extern  SEMAPHORE   goods1;
extern  SEMAPHORE   goods2;
extern  SEMAPHORE   put;
extern  SEMAPHORE   get;
extern  SEMAPHORE   space;
extern  SEMAPHORE   mutex1;
extern  SEMAPHORE   mutex2;
extern  SEMAPHORE   producer_mutex;
extern  SEMAPHORE   consumer_mutex;
