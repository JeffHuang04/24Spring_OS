
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c    全局变量定义
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"


PUBLIC	PROCESS			proc_table[NR_TASKS];       //进程表数组，NR_TASKS为最大允许进程数

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];   //任务栈

PUBLIC	TASK	task_table[NR_TASKS] = {
                    {A_Process, STACK_SIZE_A, "A_Process"},     //任务表数组
					{B_Reader, STACK_SIZE_B, "B_Reader"},
                    {C_Reader, STACK_SIZE_C, "C_Reader"},
                    {D_Reader, STACK_SIZE_D, "D_Reader"},
                    {E_Writer, STACK_SIZE_E, "E_Writer"},
                    {F_Writer, STACK_SIZE_F, "F_Writer"},
                    };
/*PUBLIC	TASK	task_table[NR_TASKS] = {
        {normalA,   STACK_SIZE_A, "A"},     //任务表数组
        {producerB, STACK_SIZE_B, "B"},
        {producerC, STACK_SIZE_C, "C"},
        {consumerD, STACK_SIZE_D, "D"},
        {consumerE, STACK_SIZE_E, "E"},
        {consumerF, STACK_SIZE_F, "F"},
};*/

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {             //系统调用表，system_call在type.h中
                                                sys_get_ticks,
                                                sys_sleep,
                                                sys_print,
                                                sys_P,
                                                sys_V
                                            };  

