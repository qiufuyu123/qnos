


[GLOBAL get_esp]
get_esp:
	mov eax,esp
	ret
[GLOBAL get_eflags]
get_eflags:
	pushf
	pop eax
	ret
[GLOBAL read_ret]
read_ret:
	mov eax,[ebp+4]
	ret
[GLOBAL switch_get]
switch_get:

	mov eax,[esp+4]     ;第一个参数 
	mov [eax],ebp
	mov [eax+4],ebx
	mov [eax+8],ecx
	mov [eax+12],edx
	mov [eax+16],esi
	mov [eax+20],edi
	mov [eax+28],esp
	push ebx
	mov ebx,eax
	pushf
	pop eax
	mov [ebx+24],eax
	mov eax,ebx
	pop ebx
	ret
[GLOBAL switch_put]
switch_put:
	mov eax,[esp+4]      ;第二个参数
	mov esp,[eax+28]    ;切换了esp    导致ret指令控制流转移
	mov ebp,[eax]
	mov ebx,[eax+4]
	mov ecx,[eax+8]
	mov edx,[eax+12]
	mov esi,[eax+16]
	mov edi,[eax+20]
	add eax,24
	push dword [eax] ;eflags
	popf	

	;由于8259a设置的手动模式 所以必须给主片与从片发送信号 否则8259a会暂停
	;这个bug找了一下午才找到 顺便吐槽下 内核级的代码debug太难了(GDB在多线程与汇编级会失效 只有print调试法) 
	mov al,0x20         
	out 0xA0,al
	out 0x20,al
	
	ret                  ;执行下一个函数
[GLOBAL switch_with_pdt]
switch_with_pdt:
	;保存上下文
	mov eax,[esp+4]     ;第一个参数 
	mov [eax],ebp
	mov [eax+4],ebx
	mov [eax+8],ecx
	mov [eax+12],edx
	mov [eax+16],esi
	mov [eax+20],edi
	mov [eax+28],esp
	push ebx
	mov ebx,eax
	pushf
	pop eax
	mov [ebx+24],eax
	mov eax,ebx
	pop ebx

	;加载上下文
	mov eax,[esp+8]      ;第二个参数
	mov esp,[eax+28]    ;切换了esp    导致ret指令控制流转移
	mov ebp,[eax]
	mov ebx,[eax+4]
	mov ecx,[eax+8]
	mov edx,[eax+12]
	mov esi,[eax+16]
	mov edi,[eax+20]
	add eax,24
	push dword [eax] ;eflags
	popf	

	;由于8259a设置的手动模式 所以必须给主片与从片发送信号 否则8259a会暂停
	;这个bug找了一下午才找到 顺便吐槽下 内核级的代码debug太难了(GDB在多线程与汇编级会失效 只有print调试法) 
	mov al,0x20         
	out 0xA0,al
	out 0x20,al
	
	ret                  ;执行下一个函数
[GLOBAL switch_to]
switch_to:
	;保存上下文
	mov eax,[esp+4]     ;第一个参数 
	mov [eax],ebp
	mov [eax+4],ebx
	mov [eax+8],ecx
	mov [eax+12],edx
	mov [eax+16],esi
	mov [eax+20],edi
	mov [eax+28],esp
	push ebx
	mov ebx,eax
	pushf
	pop eax
	mov [ebx+24],eax
	mov eax,ebx
	pop ebx

	;加载上下文
	mov eax,[esp+8]      ;第二个参数
	mov esp,[eax+28]    ;切换了esp    导致ret指令控制流转移
	mov ebp,[eax]
	mov ebx,[eax+4]
	mov ecx,[eax+8]
	mov edx,[eax+12]
	mov esi,[eax+16]
	mov edi,[eax+20]
	add eax,24
	push dword [eax] ;eflags
	popf	

	;由于8259a设置的手动模式 所以必须给主片与从片发送信号 否则8259a会暂停
	;这个bug找了一下午才找到 顺便吐槽下 内核级的代码debug太难了(GDB在多线程与汇编级会失效 只有print调试法) 
	mov al,0x20         
	out 0xA0,al
	out 0x20,al
	
	ret                  ;执行下一个函数

;global jump_usermode
test_usercode:
	cli
global jump_usermode
jump_usermode:
	;push edx
	;mov edx, [esp+4]
	mov ax, (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
	mov ds, ax
	mov es, ax 
	mov fs, ax 
	mov gs, ax ; SS is handled by iret
 
	; set up the stack frame iret expects
	mov eax, esp
	push (4 * 8) | 3 ; data selector
	push eax ; current esp
	pushf ; eflags
	push (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)
	push test_usercode ; instruction address to return to
	iret

global syscall_test
syscall_test:
	;push ebx
	;push ecx
	;push edx
	;push edi
	;mov ebx,1
	;mov ecx,2
	;mov edx,3
	;mov edi,4
	int 0x80

	;pop edi
	;pop edx
	;pop ecx
	;pop ebx
	leave
	ret
