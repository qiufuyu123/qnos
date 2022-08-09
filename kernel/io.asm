; we define some io operations and realize some hardware functions
;
global setGdt
gdtr  dw  0 
    	dd  0
setGdt:
   mov   ax, [esp + 4]
   mov   [gdtr], ax
   mov   eax, [esp + 8]
   mov   [gdtr + 2], eax
   lgdt  [gdtr]
   ret