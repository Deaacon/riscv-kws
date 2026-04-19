#include <stdint.h>

#define BIT(x) ((uint32_t) 1U << (x))
#define REG(x) ((volatile uint32_t *) (x))

#define C3_SYSTEM 0x600c0000
#define C3_GPIO 0x60004000
#define C3_SYSTIMER 0x60023000
#define C3_RTCCNTL 0x60008000
#define C3_TIMERGROUP0 0x6001F000
#define C3_TIMERGROUP1 0x60020000

enum {
    GPIO_OUT_EN = 8,
    GPIO_OUT_FUNC = 341
};

int printf(const char *format, ...);

static inline void spin(volatile unsigned long count) {
  while (count--) asm volatile("nop");
}

static inline uint64_t systick(void) {
    REG(C3_SYSTIMER)[1] = BIT(30);
    spin(1);
    return ((uint64_t) REG(C3_SYSTIMER)[16] << 32) | REG(C3_SYSTIMER)[17];
}

static inline uint64_t uptime_us(void) {
    return systick() >> 4;
}

static inline void delay_us(unsigned long us) {
    uint64_t until = uptime_us() + us;
    while (uptime_us() < until) spin(1);
}

static inline void delay_ms(unsigned long ms) {
    delay_us(ms * 1000);
}

static inline void wdt_disable(void) {
    REG(C3_RTCCNTL)[42] = 0x50d83aa1;
    REG(C3_RTCCNTL)[36] = 0;
    REG(C3_RTCCNTL)[35] = 0;

    REG(C3_RTCCNTL)[44] = 0x8F1D312A;
    REG(C3_RTCCNTL)[43] |= BIT(31);
    REG(C3_RTCCNTL)[45] = 0;

    REG(C3_TIMERGROUP0)[63] &= ~BIT(9);
    REG(C3_TIMERGROUP0)[18] = 0;
    REG(C3_TIMERGROUP1)[18] = 0;
}

static inline void soc_init(void) {
    REG(C3_SYSTEM)[2] &= ~3U;
    REG(C3_SYSTEM)[2] |= BIT(0) | BIT(2);
    REG(C3_SYSTEM)[22] = BIT(19) | (40U << 12) | BIT(10);
    ((void (*)(int)) 0x40000588)(160);
}

static inline void gpio_output_enable(int pin, int enable) {
    REG(C3_GPIO)[GPIO_OUT_EN] &= ~BIT(pin);
    REG(C3_GPIO)[GPIO_OUT_EN] |= (enable ? 1U : 0U) << pin;
}

static inline void gpio_output(int pin) {
    REG(C3_GPIO)[GPIO_OUT_FUNC + pin] = BIT(9) | 128;
    gpio_output_enable(pin, 1);
}

static inline void gpio_write(int pin, int value) {
    REG(C3_GPIO)[1] &= ~BIT(pin);
    REG(C3_GPIO)[1] |= (value ? 1U : 0U) << pin;
}

static inline void gpio_toggle(int pin) {
    REG(C3_GPIO)[1] ^= BIT(pin);
}
