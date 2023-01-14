
global _start
MB
OOT_PAGE_ALIGN    equ 1<<0		; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1		; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002	; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ 7
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


;global .mboot
section .mboot
align 4
    dd  MBOOT_HEADER_MAGIC	; GRUB will search for this value on each
				; 4-byte boundary in your kernel file
    dd  MBOOT_HEADER_FLAGS	; How GRUB should load your file / settings
    dd  MBOOT_CHECKSUM		; To ensure that the above values are correct
    dd  0                   ;header addr
    dd  0                   ;load addr
    dd  0                   ;load end addr
    dd  0                   ;bss
    dd  0                   ;entry
    dd  0                   ;mode type
    dd  800                ;width
    dd  600                ;height
    dd  32                  ;depth
    resb 4*13
extern kernelmain

;为内核栈空出16k地址
section .bss
align 4
stack_bottom:
resb 42767 ; 32 KiB

stack_top:

section .text
align 4

;.type _start, @function
_start:
    ;通过设置esp的值设置stack
    mov esp, stack_top
    ;压入multiboot 头
    push    ebx
    push    eax
    ;进入c内核
    call kernelmain
    jmp $
    