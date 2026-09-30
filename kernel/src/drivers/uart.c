#include <minemu/platform.h>
#include <minemu/uart.h>
#include <minemu/irq.h>
#include <minemu/irq_table.h>

#define RX_BUFFER_SIZE 256u

void uart_putc(char c){
    while (!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY));
    MINEMU_UART0->tx_data = (uint32_t)(uint8_t)c;
}

void uart_write(const char *buf, size_t len){
    for (size_t i = 0; i < len; i++){
        uart_putc(buf[i]);
    }
}

void uart_puts(const char *str){
    while (*str){
        uart_putc(*str++);
    }
}

static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;

static inline uint32_t irq_save(void) {
    uint32_t cpsr;
    __asm__ volatile("mrs %0, cpsr" : "=r"(cpsr) : : "memory");
    minemu_irq_disable();
    return cpsr;
}

static inline void irq_restore(uint32_t cpsr) {
    if (!(cpsr & 0x80u)) {
        minemu_irq_enable();
    }
}

static void uart_rx_irq_handler(void) {
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint8_t data = (uint8_t)MINEMU_UART0->rx_data;
        uint32_t next_head = (rx_head + 1u) & (RX_BUFFER_SIZE - 1u);
        if (next_head != rx_tail) {
            rx_buffer[rx_head] = data;
            rx_head = next_head;
        }
    }
}

int uart_getc_nb(void) {
    int c = -1;
    uint32_t primask = irq_save();
    if (rx_head != rx_tail) {
        c = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1u) & (RX_BUFFER_SIZE - 1u);
    }
    irq_restore(primask);
    return c;
}

void uart_init(void) {
    minemu_irq_register(MINEMU_IRQ_UART0, uart_rx_irq_handler);
    MINEMU_UART0->control = MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
    MINEMU_INTERRUPT->enable |= UINT32_C(1) << MINEMU_IRQ_UART0;
}
