
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h  函数声明
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);
PUBLIC void disp_int(int input);
void disable_irq(int irq);
void enable_irq(int irq);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
//  ADD:
void B_Reader();
void C_Reader();
void D_Reader();
void E_Writer();
void F_Writer();
void A_Process();
PUBLIC void Reader(char* name,int proc_ticks,int num);
PUBLIC void Writer(char* name,int proc_ticks,int num);

void normalA();
void producerB();
void producerC();
void consumerD();
void consumerE();
void consumerF();

char* int_string(int num);
void clear_screen();
void print_int(int num);

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);
PUBLIC void init_8259A();

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void milli_delay(int milli_sec);


/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
//ADD
PUBLIC  void    milli_sleep(int milli_sec);
PUBLIC  void    p_semaphore(SEMAPHORE* semaphore);
PUBLIC  void    v_semaphore(SEMAPHORE* semaphore);


/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
//ADD
PUBLIC  void    sleep(int milli_sec);
PUBLIC  void    sys_sleep(int milli_sec);
PUBLIC  void    print(char* str);
PUBLIC  void    sys_print(char* str);
PUBLIC  void    P(SEMAPHORE* semaphore);
PUBLIC  void    sys_P(SEMAPHORE* semaphore);
PUBLIC  void    V(SEMAPHORE* semaphore);
PUBLIC  void    sys_V(SEMAPHORE* semaphore);
