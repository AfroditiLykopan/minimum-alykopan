#include <stddef.h>
#include <stdint.h>
#include <minemu/msh.h>
#include <minemu/uart.h>

#define MSH_LINE_MAX 20
#define MSH_PROMPT "msh> "

static int word_equals(const char *a, size_t a_len, const char *lit) {
    size_t i = 0;
    while (lit[i] != '\0') {
        if (i >= a_len || a[i] != lit[i]) {
            return 0;
        }
        i++;
    }
    return i == a_len;
}

static void msh_execute(const char *line, size_t line_len) {
    size_t i = 0;

    while (i < line_len && line[i] == ' ') {
        i++;
    }
    if (i == line_len) {
        return;
    }

    size_t cmd_start = i;
    while (i < line_len && line[i] != ' ') {
        i++;
    }
    size_t cmd_len = i - cmd_start;

    if (word_equals(line + cmd_start, cmd_len, "echo")) {
        while (i < line_len && line[i] == ' ') {
            i++;
        }
        uart_write(line + i, line_len - i);
        uart_putc('\n');
    } else {
        uart_puts("command not found: ");
        uart_write(line + cmd_start, cmd_len);
        uart_putc('\n');
    }
}

void msh_run(void) {
    char line[MSH_LINE_MAX];
    size_t line_len = 0;
    size_t overflow = 0;

    uart_puts(MSH_PROMPT);
    for (;;) {
        int c = uart_getc_nb();
        if (c < 0) {
            continue;
        }

        if (c == '\n') {
            msh_execute(line, line_len);
            line_len = 0;
            overflow = 0;
            uart_puts(MSH_PROMPT);
        } else if (c == 0x08 || c == 0x7f) {
            if (overflow > 0) {
                overflow--;
            } else if (line_len > 0) {
                line_len--;
            }
        } else if (line_len < MSH_LINE_MAX) {
            line[line_len++] = (char)c;
        } else {
            overflow++;
        }
    }
}
