#ifndef MINEMU_UART_H
#define MINEMU_UART_H
#include <stdint.h>
#include <stddef.h>

void uart_init(void);
void uart_putc(char c);
void uart_write(const char *buf, size_t len);
void uart_puts(const char *str);
int uart_getc(void);
void uart_rx_irq_handler(void);
#endif