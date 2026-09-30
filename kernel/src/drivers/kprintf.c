#include <stdint.h>
#include <stdarg.h>
#include <minemu/uart.h>

static void put_uint(uint32_t value, unsigned base, int upper){
    char tmp[32];
    int i = 0;
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (value == 0){
        tmp[i++] = '0';
    }
    while (value){
        tmp[i++] = digits[value % base];
        value /= base;
    }
    while (i--){
        uart_putc(tmp[i]);
    }
}

void kprintf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { uart_putc(*fmt); continue; }
        switch (*++fmt) {
            case 'c': uart_putc((char)va_arg(args, int)); break;
            case 's': { const char *str = va_arg(args, const char *);
                        uart_puts(str ? str : "(null)"); break; }
            case 'd': { int32_t value = va_arg(args, int32_t);
                        if (value < 0) { uart_putc('-'); put_uint(-(uint32_t)value, 10, 0); }
                        else put_uint((uint32_t)value, 10, 0); break; }
            case 'u': put_uint(va_arg(args, uint32_t), 10, 0); break;
            case 'x': put_uint(va_arg(args, uint32_t), 16, 0); break;
            case 'X': put_uint(va_arg(args, uint32_t), 16, 1); break;
            case '%': uart_putc('%'); break;
            default: uart_putc('%'); uart_putc(*fmt); break; }
            
    }
    va_end(args);
}