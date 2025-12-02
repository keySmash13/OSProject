typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long  uint64_t;

#define UART0_BASE 0x10000000
#define UART_TX    0x00
#define UART_RX    0x00
#define UART_LSR   0x05  /* Line status register */
#define UART_LSR_DR 0x01 /* Data ready */

static inline void uart_putc(char c) {
    volatile uint8_t *tx = (volatile uint8_t *)(UART0_BASE + UART_TX);
    *tx = c;
}

static inline void uart_puts(const char *s) {
    for (const char *p = s; *p; ++p) uart_putc(*p);
}

static inline char uart_getc(void) {
    volatile uint8_t *lsr = (volatile uint8_t *)(UART0_BASE + UART_LSR);
    volatile uint8_t *rx  = (volatile uint8_t *)(UART0_BASE + UART_RX);
    while (!(*lsr & UART_LSR_DR))
        ; /* wait until data ready */
    return *rx;
}

// read a string from UART
void strin(char dest[], int len) {
    unsigned char chr;
    int i = 0;

    for (;;) {
        chr = uart_getc();

        switch(chr) {
            case '\r':
            case '\n':
                dest[i] = '\0';
                uart_puts("\r\n");
                return;
            case 0x7f: // backspace
            case 0x08:
                if (i > 0) {
                    uart_puts("\b \b");
                    i--;
                }
                break;
            default:
                if (i < len - 1) {
                    dest[i++] = chr;
                    uart_putc(chr);
                }
        }
    }
}

// simple string compare
int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) a++, b++;
    return (unsigned char)*a - (unsigned char)*b;
}

// simple string compare for prefix
int strncmp(const char *a, const char *b, unsigned int n) {
    unsigned int i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0')
            return (unsigned char)a[i] - (unsigned char)b[i];
    }
    return 0;
}

// simple command functions
void cmd_help(void) {
    uart_puts("Available commands:\n");
    uart_puts("  help - Show this help message\n");
    uart_puts("  echo <text> - Echo text back\n");
    uart_puts("  ls - List dummy files\n");
}

void cmd_echo(char *args) {
    uart_puts(args);
    uart_puts("\n");
}

void cmd_ls(void) {
    uart_puts("file1.txt  file2.txt  kernel.bin\n");
}

// command dispatcher
void run_command(char *input) {
    // skip leading spaces
    while (*input == ' ') input++;

    if (strncmp(input, "help", 4) == 0) {
        cmd_help();
    } else if (strncmp(input, "echo", 4) == 0) {
        char *args = input + 4;
        while (*args == ' ') args++;
        cmd_echo(args);
    } else if (strcmp(input, "ls") == 0) {
        cmd_ls();
    } else if (*input != '\0') {
        uart_puts("Unknown command. Type 'help' for a list.\n");
    }
}

void kmain(void) {
    uart_puts("Please look at this window for input/output!\n");
    uart_puts("tiny-rv64-kernel: ready!\n");

    char buffer[100];

    for (;;) {
        uart_puts("> ");      // prompt
        strin(buffer, 100);   // read input
        run_command(buffer);  // execute
    }
}
