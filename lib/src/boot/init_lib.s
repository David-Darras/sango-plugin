@ Sets up the heap of the plugin and runs the constructors of the static
@ objects. This file is from libctrpf (stack_adjust.s).
@
@ The constructors run on the stack of the boot thread (see boot.cc). The
@ stack must stay 8-aligned at each call, so the functions push two registers.

    .arm
    .align 2

    .global initLib
    .type   initLib, %function
initLib:
    push    {r4, lr}
    bl      __system_allocateHeaps
    bl      __libc_init_array
    pop     {r4, lr}
    bx      lr

    .global __ctru_exit
    .type   __ctru_exit, %function
@ Closes the services and runs the destructors.
__ctru_exit:
    push    {r4, lr}
    bl      __appExit
    bl      __libctru_exit
    bl      __libc_fini_array
    pop     {r4, lr}
    bx      lr
