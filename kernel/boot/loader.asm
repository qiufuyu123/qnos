
global _start
MBOOT_PAGE_ALIGN    equ 1<<0		; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1		; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002	; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


;global .mboot
section .mboot
align 4
    dd  MBOOT_HEADER_MAGIC	; GRUB will search for this value on each
				; 4-byte boundary in your kernel file
    dd  MBOOT_HEADER_FLAGS	; How GRUB should load your file / settings
    dd  MBOOT_CHECKSUM		; To ensure that the above values are correct
extern kernelmain

;为内核栈空出16k地址
section .bss
align 4
stack_bottom:
resb 17384 ; 16 KiB
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
    