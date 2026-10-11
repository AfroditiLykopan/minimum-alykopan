#include <stdint.h>
#include <stddef.h>
#include "minemu/user_abi.h"

#define LINE_MAX_LEN 64U

static void out(const char *str, uint32_t len) {
    while (len != 0U) {
        int32_t result = ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, (void *)str, len);
        if (result <= 0) {
            return;
        }
        str += result;
        len -= (uint32_t)result;
    }
} 

static int read_byte(void) {
    char byte;
    for (;;) {
        int32_t len = ioctl(MINEMU_FD_UART_IN, MINEMU_IOCTL_UART_READ, (void *)&byte, 1U);
        if (len > 0) {
            return (int)(unsigned char)byte;
        }
    }
}

static int word_is(const char *word, size_t len, const char *lit) {
    size_t i = 0;
    while (lit[i] != '\0') {
        if (i >= len || word[i] != lit[i]) {
            return 0;
        }
        i++;
    }
    return i == len;
}

static void run_command(const char *cmd, size_t len) {
    size_t i = 0;
    while ( i < len && cmd[i] == ' ') {
        i++;
    }
    if (i == len) {
        return;
    }
    size_t j = i;
    while (i < len && cmd[i] != ' ') {
        i++;
    }
    size_t cmd_len = i - j;

       if (word_is(&cmd[j], cmd_len, "echo")) {
        int first = 1;
        while (i < len) {
            while (i < len && cmd[i] == ' ') {
                i++;
            }
            if (i >= len) {
                break;
            }
            size_t s = i;
            while (i < len && cmd[i] != ' ') {
                i++;
            }
            if (!first) {
                out(" ", 1U);
            }
            out(&cmd[s], (uint32_t)(i - s));
            first = 0;
        }
        out("\n", 1U);
        return;
    }

    out("command not found: ", 19U);
    out(&cmd[j], (uint32_t)cmd_len);
    out("\n", 1U);
}

static char prompt[] = "msh> ";

void minemu_user_main(void) {
    char line[LINE_MAX_LEN];

    for (;;) {
        out(prompt, sizeof(prompt) - 1);
        size_t len = 0;
        size_t offset = 0;

        for (;;) {
            int byte = read_byte();
            if (byte == '\n') {
                break;
            }
            if (byte == 0x08 || byte == 0x7f) {
                if (offset > 0U) {
                    offset--;
                } else if (len > 0U) {
                    len--;
                }
                continue;
            }
            if (len < LINE_MAX_LEN) {
                line[len++] = (char)byte;
            } else {
                offset++;
            }
        }
        if (offset == 0U) {
        run_command(line, len); }
    }
}
