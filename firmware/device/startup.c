#include <stdint.h>

extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern uint32_t _stack_top;

void main(void);

__attribute__((section(".entry_addr")))
void _start(void) {
    __asm__ volatile ("mv sp, %0" : : "r"(&_stack_top));

    uint8_t *bss = (uint8_t*)&_bss_start;
    while (bss < (uint8_t*)&_bss_end) {
        *bss++ = 0;
    }

    main();

    while (1) {
        __asm__ volatile ("nop");
    }
}
