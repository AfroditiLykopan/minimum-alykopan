#include "minemu/irq.h"
#include "minemu/syscall.h"
#include "minemu/platform.h"
#include "minemu/irq_table.h"

void minemu_fail_stop(void) {
    for (;;) {
        __asm__ volatile("nop");
    }
}

void minemu_panic(const char *message) {
    (void)message;
    minemu_fail_stop();
}

__attribute__((weak, noreturn)) void minemu_svc_trampoline(void) {
    minemu_fail_stop();
}

__attribute__((weak, noreturn)) void minemu_irq_trampoline(void) {
    minemu_fail_stop();
}

__attribute__((weak)) struct minemu_trap_frame *minemu_svc_dispatch(
    struct minemu_trap_frame *frame) {
    (void)frame;
    minemu_fail_stop();
}

__attribute__((weak)) void minemu_undefined_dispatch(struct minemu_trap_frame *frame) {
    (void)frame;
    minemu_fail_stop();
}

__attribute__((weak)) void minemu_abort_dispatch(struct minemu_trap_frame *frame) {
    (void)frame;
    minemu_fail_stop();
}

#define MINEMU_IRQ_COUNT 4u

static minemu_irq_handler_t irq_handlers[MINEMU_IRQ_COUNT];

void minemu_irq_register(uint32_t id, minemu_irq_handler_t handler) {
    if (id < MINEMU_IRQ_COUNT) {
        irq_handlers[id] = handler;
    }
}

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t id = (uint32_t)frame->exception_id;

    if (id < MINEMU_IRQ_COUNT && irq_handlers[id] != 0) {
        irq_handlers[id]();
    }
    if (id < MINEMU_IRQ_COUNT) {
        MINEMU_INTERRUPT->eoi = id;
    }
    return frame;
}
