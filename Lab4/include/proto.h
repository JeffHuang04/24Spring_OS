
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
PUBLIC void disable_int();
PUBLIC void enable_int();

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void NormalA();
void B();
void C();
void D();
void E();
void F();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);
PUBLIC void init_screen(TTY* p_tty);

/* printf.c */
PUBLIC  int     printf(const char *fmt, ...);

/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC  void    sys_output_str(char* str, int len);
PUBLIC  void    sys_sleep(int milli_sec, int type);
PUBLIC  void    sys_P(SEMAPHORE* semaphore);
PUBLIC  void    sys_V(SEMAPHORE* semaphore);
/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC  void    output_str(char* str, int len);
PUBLIC  void    sleep(int milli_sec, int type);
PUBLIC  void    P(SEMAPHORE* semaphore);
PUBLIC  void    V(SEMAPHORE* semaphore);