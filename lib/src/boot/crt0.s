@ The start of the plugin. The plugin loader (Luma3DS) calls _start from the
@ thread of the game, before the game runs its own code. This file is from
@ libctrpf (3gx_crt0.s).

    .cpu mpcore

    .section ".crt0"
    .global _start, __service_ptr
    .align 4
    .arm

_start:
    b startup
__service_ptr:
    .word 0 @ Pointer to the service handle override list (libctru)
startup:
    @ Save the return address
    mov     r4, lr

    @ Clear the BSS section
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    sub     r1, r1, r0
    bl      ClearMem

    @ r0: the saved instructions of the game, r1: a temporary stack
    ldr     r0, [sp]
    sub     r1, sp, #0x100
    bl      __entrypoint

    @ Go back to the plugin loader
    mov     pc, r4

@ Clears the memory to 0
@   r0 = start address
@   r1 = length
ClearMem:
    mov     r2, #3     @ Round down to a word
    add     r1, r1, r2
    bics    r1, r1, r2
    bxeq    lr         @ Nothing to clear

    mov     r2, #0
ClearLoop:
    stmia   r0!, {r2}
    subs    r1, r1, #4
    bne     ClearLoop

    bx      lr
