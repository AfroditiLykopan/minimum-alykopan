.syntax unified
.cpu cortex-a9
.arm

.global minemu_irq_trampoline
.type minemu_irq_trampoline, %function
.extern minemu_irq_dispatch

minemu_irq_trampoline:
    /* 1. allocate one 88-byte trap frame on the IRQ stack */
    sub sp, sp, #88

    /* 2. save r0-r12, the raw IRQ lr, and the interrupted spsr */
    stmia sp, {r0-r12}
    str lr, [sp, #52]
    mrs r1, spsr
    str r1, [sp, #56]

    /* 3. ask the interrupt controller which source fired */
    ldr r1, =0x10000000
    ldr r1, [r1, #8]
    str r1, [sp, #60]

    /* 4. address of the interrupted instruction = lr - 4 */
    sub r1, lr, #4
    str r1, [sp, #64]

    /* unused fields for IRQs */
    mov r1, #0
    str r1, [sp, #68]
    str r1, [sp, #72]

    /* 5. save the user-mode banked sp and lr */
    add r1, sp, #76
    stmia r1, {sp, lr}^
    mov r1, #0
    str r1, [sp, #84]

    /* 6. call the C dispatcher; it returns the frame to resume */
    mov r0, sp
    bl minemu_irq_dispatch

    /* 7. restore from the RETURNED frame */
    mov sp, r0
    ldr lr, [sp, #52]
    ldr r1, [sp, #56]
    msr spsr_cxsf, r1
    add r1, sp, #76
    ldmia r1, {sp, lr}^
    ldmia sp, {r0-r12}
    add sp, sp, #88

    /* 8. return to the interrupted instruction and restore CPSR from SPSR */
    subs pc, lr, #4
