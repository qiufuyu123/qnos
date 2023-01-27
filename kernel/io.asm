; we define some io operations and realize some hardware functions
;
global setGdt
global load_eflags
global store_eflags
gdtr  dw  0 
    	dd  0
setGdt:
   mov   ax, [esp + 4]
   mov   [gdtr], ax
   mov   eax, [esp + 8]
   mov   [gdtr + 2], eax
   lgdt  [gdtr]
   ret
load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS
		POP		EAX
		RET
store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS
		RET
