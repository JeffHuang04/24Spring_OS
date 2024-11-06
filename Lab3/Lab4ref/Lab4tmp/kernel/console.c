
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PUBLIC void exit_search(CONSOLE* p_con);
PUBLIC void search(CONSOLE* p_con);
/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	init_pos_stack(p_tty->p_console);//初始化位置栈
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case '\n':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {
			push_pos(p_con, p_con->cursor);
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
		}
		break;
	case '\b'://删除
		if (p_con->cursor > p_con->original_addr) {
			int now = p_con->cursor;
			int last = pop_pos(p_con);
			p_con->cursor = last;
			for(int i = 0; i < now - last;i++){
				*(p_vmem-2-2*i) = ' ';
				*(p_vmem-1-2*i) = DEFAULT_CHAR_COLOR;
			}
		}
		break;
	case '\t':
		if (p_con->cursor < p_con->original_addr + 
			p_con->v_mem_limit - TAB_WIDTH) {
			//p_con->original_addr 控制台显示区域的起始地址 
			//p_con->v_mem_limit 显示区上限地址
			push_pos(p_con, p_con->cursor);
			for(int i=0;i<TAB_WIDTH;++i){ // 用蓝色空格填充
 				*p_vmem++ = ' ';
 				*p_vmem++ = BLUE;
 			}
			p_con->cursor += TAB_WIDTH;//需要移动光标
		}
		break;
	case '\01':
		p_con->color = RED;
		break;
	case '\02':
		p_con->color = GREEN;
		break;
	case '\03':
		p_con->color = BRIGHT_BLUE;
		break;
	case '\04':
		p_con->color = BRIGHT_RED;
		break;
	case '\05':
		p_con->color = BRIGHT_GREEN;
		break;
	case '\06':
		p_con->color = DEFAULT_CHAR_COLOR;
		break;
	// default:
	// 	if (p_con->cursor <
	// 	    p_con->original_addr + p_con->v_mem_limit - 1) {
	// 		*p_vmem++ = ch;
	// 		if(mode == 1 ){
	// 			*p_vmem++ = RED;
	// 		}else{
	// 			*p_vmem++ = DEFAULT_CHAR_COLOR;
	// 		}
	// 		push_pos(p_con, p_con->cursor);
	// 		p_con->cursor++;
	// 	}
	// 	break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = p_con->color;
			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

PUBLIC void exit_search(CONSOLE* p_con){
	u8 *vmem = (u8 *)V_MEM_BASE; // 显存基地址
	u8* p_vmem = (u8*)(vmem + p_con->cursor * 2);//计算当前光标位置
	int search_length = p_con->cursor - p_con->search_start_cursor;
	for(int i = 0; i < search_length;i++){
		*(p_vmem - 2 - 2 * i) = ' ';
        *(p_vmem - 1 - 2 * i) = DEFAULT_CHAR_COLOR;
	}//从后向前清除搜索的内容
	for(int i=0;i<p_con->search_start_cursor*2;i += 2){
		if(*(vmem + i + 1)!=BLUE){
			*(vmem + i + 1) = DEFAULT_CHAR_COLOR;
		}
	}//把搜索变红内容变白且不改变TAB的蓝色
	p_con->cursor = p_con->search_start_cursor;//重置光标
	p_con->pos_stack.cursorPos = p_con->pos_stack.searchCursorPos;//重置栈指针
	flush(p_con);//刷新控制台
}

PUBLIC void search(CONSOLE* p_con){
	int search_length = (p_con->cursor - p_con->search_start_cursor)*2;//搜索词的长度包括颜色
	u8 *vmem = (u8 *)V_MEM_BASE; // 显存基地址
	for(int i = 0; i< p_con->search_start_cursor*2;i+=2){
		int flag = 1;
		for(int j = 0;j < search_length;j+=2){
			if (*(vmem + i +j) != *(vmem + p_con->search_start_cursor*2 + j)
			||(*(vmem + i + j + 1)==BLUE && *(vmem + p_con->search_start_cursor*2 + j+1) != BLUE)
			||(*(vmem + i + j + 1)!=BLUE && *(vmem + p_con->search_start_cursor*2 + j+1) == BLUE)){
				flag = 0;
				break;
			}
		}
		if(flag){
			for(int j = 0; j < search_length;j+=2){
				if(*(vmem + i + j + 1)!=BLUE){
					*(vmem + i + j + 1) = RED;
				}
			}
		}
	}
}