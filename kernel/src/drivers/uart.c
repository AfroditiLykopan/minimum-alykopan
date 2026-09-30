#include <minemu/platform.h>
#include <minemu/uart.h>

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