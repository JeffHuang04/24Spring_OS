
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);


/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */   
//ADD
PUBLIC  void     sys_print(char* str);
PUBLIC  void     sys_sleep(int milli_sec);
PUBLIC  void     sys_P(SEMAPHORE* semaphore);
PUBLIC  void     sys_V(SEMAPHORE* semaphore);
/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
//ADD
PUBLIC  void    print(char* str);
PUBLIC  void    sleep(int milli_sec);
PUBLIC  void    P(SEMAPHORE* semaphore);
PUBLIC  void    V(SEMAPHORE* semaphore);

