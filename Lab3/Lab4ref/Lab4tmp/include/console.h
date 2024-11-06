
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_



#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80
#define TAB_WIDTH 4

typedef struct cursor_stack{
	int cursorPos;
	int pos[SCREEN_SIZE];
	int searchCursorPos;
}STACK;//存每一个位置的栈
/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
	unsigned int search_start_cursor;		//搜索模式开始时光标的位置
	STACK pos_stack;
	u8	color;
}CONSOLE;

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */
PRIVATE void init_pos_stack(CONSOLE *p_con);
PRIVATE void push_pos(CONSOLE *p_con, int pos);
PRIVATE int pop_pos(CONSOLE *p_con);

PRIVATE void init_pos_stack(CONSOLE *p_con){
	p_con->pos_stack.cursorPos = 0;
	p_con->pos_stack.searchCursorPos = 0;
}
PRIVATE void push_pos(CONSOLE* p_con,int pos){
 	p_con->pos_stack.pos[p_con->pos_stack.cursorPos] = pos;
	p_con->pos_stack.cursorPos++;
}
PRIVATE int pop_pos(CONSOLE* p_con){
 	if(p_con->pos_stack.cursorPos == 0){
 		return 0;
 	}else{
		p_con->pos_stack.cursorPos--;
 		return p_con->pos_stack.pos[p_con->pos_stack.cursorPos];
 	}
}
#endif /* _ORANGES_CONSOLE_H_ */
