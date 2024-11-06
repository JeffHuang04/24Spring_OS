
section .data
    inputnum: times 35 dq 0; 输入数字内容（数据范围是10^30）
    numlen: dq 0; 输入数字长度
    inputbase: dq 0; 输入数字进制
    outputbase: dq 0; 输出进制标识
    tmp: dq 0; 输入输出的临时存储
    outputerror: db "Error",10; 输出错误信息且换行
    dest: dq 0
    remain: dq 0
    outcome: times 120 dq 0;输出结果(未颠倒)
    outcomereverse: times 120 dq 0;输出结果
    outlen: dq 0;
section .bss

section .text
    global main
    global _start

;main:
_start:
    ONCE:
    call read_num
    call read_base
    call clear_buffer
    call change
    call print_outcome
    call putenter
    mov qword[tmp],0;清除缓冲
    jmp ONCE
    call exit

getchar:; 读取单个字符
    mov rax, 0
    mov rdi, 0
    mov rsi, tmp
    mov rdx, 1
    syscall
    cmp rax, 1
    je notEOF
    mov qword [tmp], -1
    notEOF:
    ret

putchar:; 输出单个字符
    mov rax, 1
    mov rdi, 1
    mov rsi, tmp
    mov rdx, 1
    syscall
    ret

putenter:; 输出回车
    mov qword [tmp], 10
    call putchar
    ret

puterror:; 输出错误
    mov rax, 1
    mov rdi, 1
    mov rsi, outputerror
    mov rdx, 6
    syscall
    call clear_buffer
    jmp ONCE
    ret

read_num:
    mov rbp, inputnum; 将inputnum传参给rbp
    mov rbx, 0; 作为数组的下标
    READ1:
        call getchar
        cmp qword[tmp], '0'; 如果小于‘0’，则结束循环
            jl READ_END1
        cmp qword[tmp], '9'; 如果大于‘9’，则结束循环
            jg READ_END1
        sub qword[tmp], '0'; 将ascii码转换成数字
        mov r11, qword[tmp]
        mov qword[rbp+rbx*8], r11; 存进数组
        inc rbx; 数组下标加一
        jmp READ1
    READ_END1:
        cmp qword[tmp], 'q'; 输入q退出程序
            je exit
        cmp qword[tmp], 10; 换行则正常退出 
            je COMMOM_END1
        cmp qword[tmp], 13; 回车则正常退出
            je COMMOM_END1
        cmp qword[tmp], 32; 空格则正常退出
            je COMMOM_END1
        jmp ERROR_END1; 遇到其他字符则错误退出
    ERROR_END1: call puterror
    COMMOM_END1:
    mov qword[numlen], rbx;返回输入数字的长度
    ret

clear_buffer:
    mov rax, 0       ; 使用系统调用read来清空输入缓冲区
    mov rdi, 0       ; 标准输入文件描述符为0
    mov rsi, tmp     ; 缓冲区地址
    mov rdx, 1     ; 缓冲区大小
    syscall
    ret

read_base:; 读取进制
    READ2:
        call getchar
        cmp qword[tmp], 'b'
            je BASE2
        cmp qword[tmp], 'o'
            je BASE8
        cmp qword[tmp], 'h'
            je BASE16
        cmp qword[tmp], 'q'; 输入q退出程序
            je exit
        jmp ERROR_END2
    BASE2:
        mov qword[inputbase], 2
        mov qword[outputbase], 'b'
        ret
    BASE8:
        mov qword[inputbase], 8
        mov qword[outputbase], 'o'
        ret
    BASE16:
        mov qword[inputbase], 16
        mov qword[outputbase], 'x'
        ret
    ERROR_END2: 
        call puterror
        ;jmp ONCE
    ret

change:
    mov rbp, inputnum; 传参给rbp
    mov rbx, 0; 输入数字的计数器
    mov rcx, 0; 输出数字的计数器
    mov r10, 0; 检查全零的寄存器
    CHECKALLZERO:
        cmp r10, qword[numlen]
            je REVERSE
        cmp qword[rbp+r10*8], 0
            jne CHANGE
        inc r10; 检查全零寄存器递增
        jmp CHECKALLZERO
    CHANGE:
        mov r10, 0; 全零寄存器归零
        cmp rbx, qword[numlen]
            je ADDCHAR
        mov r8, 10
        mov rax, qword[remain]; 被乘数存在rax中
        mul r8; 只显示乘数即可
        mov r9, rax; 结果存储在rax转移到r9
        add r9, qword[rbp+rbx*8]
        mov qword[dest],r9
        mov rax, r9; 将被除数存在rax中
        mov r8, qword[inputbase]
        mov rdx, 0; 将rdx清零
        div r8; 只显示除数
        mov qword[remain], rdx; 余数存放在rdx中
        mov qword[rbp+rbx*8], rax; 除数放在rax中
        inc rbx
        jmp CHANGE
    ADDCHAR:; 添加字符
        mov rbx, 0;
        mov r9, qword[remain]
        mov qword[outcome+rcx*8],r9
        inc rcx
        mov qword[remain], 0
        jmp CHECKALLZERO
    REVERSE:; 将结果反转
        mov qword[outlen], rcx; 将输出字符的长度储存
        mov rbx, qword[outlen]
        dec rbx
        mov rcx, 0
        loop1:
            mov r11,qword[outcome+rbx*8]
            mov qword[outcomereverse+rcx*8],r11
            dec rbx
            inc rcx
            cmp rcx, qword[outlen]
                jl loop1
            jmp CHANGEEND
    CHANGEEND:
    ret

print_outcome:
    mov rbx, 0
    mov rbp, outcomereverse
    call print_base
    PRINT:
        mov r11, qword[rbp+rbx*8]
        mov qword[tmp],r11
        call changebit
        call putchar
        inc rbx
        cmp rbx,qword[outlen] 
            jl PRINT
    ret

changebit:
    cmp qword[tmp],9
        jle CHANGE1
    add qword[tmp],87;a是97
    jmp CHANGEBITEND
    CHANGE1: 
        add qword[tmp],'0'
    CHANGEBITEND:
    ret

print_base:
    mov qword[tmp],'0'
    call putchar
    mov r11,qword[outputbase]
    mov qword[tmp],r11
    call putchar
    ret

exit:
    mov rax, 60
    mov rdi, 0
    syscall
    ret