global myprint

length:;计算传入字符长度
    push ebx;保存ebx
    mov ebx, eax;将开始地址赋给ebx
    next:
        cmp byte[eax], 0;判断是否是空白符
        je end
        inc eax;读取下一个字符
        jmp next
    end:
        sub eax, ebx;得到字符串长度，存在eax中
        pop ebx;恢复ebx
        ret

myprint:
    mov eax, [esp+4];将参数传递给eax
    push eax;保存eax中的值
    call length;获得字符串长度
    mov edx, eax;将长度保存在edx中
    pop eax;恢复eax的值
    mov ecx, eax;ecx保存打印字符串起始地址
    mov eax, 4;打印
    mov ebx, 1;打印
    int 80h;系统调用
    ret