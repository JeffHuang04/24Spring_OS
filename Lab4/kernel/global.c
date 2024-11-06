
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	PROCESS	proc_table[NR_TASKS + NR_PROCS];

PUBLIC	TASK	task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "tty"}};

PUBLIC  TASK    user_proc_table[NR_PROCS] = {
	{NormalA, STACK_SIZE_TESTA, "NormalA"},
	{B, STACK_SIZE_TESTB, "B"},
	{C, STACK_SIZE_TESTC, "C"},
	{D, STACK_SIZE_TESTD, "D"},
	{E, STACK_SIZE_TESTE, "E"},
	{F, STACK_SIZE_TESTF, "F"}};

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks,
												sys_output_str,
												sys_sleep,
												sys_P,
												sys_V
												};

PUBLIC int mode;

PUBLIC  SEMAPHORE rw_mutex = {1, 0, 0};
PUBLIC  SEMAPHORE writer_mutex = {1, 0, 0};
PUBLIC  SEMAPHORE reader_mutex = {1, 0, 0};
PUBLIC  SEMAPHORE queue = {1, 0, 0};
PUBLIC  SEMAPHORE readerNum_mutex = {MAX_READERS, 0, 0};



PUBLIC  SEMAPHORE   goods1 = {0,0,0};
PUBLIC  SEMAPHORE   goods2 = {0,0,0};
PUBLIC  SEMAPHORE   put = {0,0,0};
PUBLIC  SEMAPHORE   get = {0,0,0};
PUBLIC  SEMAPHORE   space = {SPACE_CAPACITY,0,0};
PUBLIC  SEMAPHORE   mutex1 = {1,0,0};
PUBLIC  SEMAPHORE   mutex2 = {2,0,0};
PUBLIC  SEMAPHORE   producer_mutex = {1,0,0};
PUBLIC  SEMAPHORE   consumer_mutex = {1,0,0};