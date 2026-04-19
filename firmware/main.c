#include <mdk.h>

static int led_pin = 8;
static int led_state = 0;

int main(void) {
    wdt_disable();

    printf("Hello World!\n");
    gpio_output(led_pin);

    for (;;) {
        printf("LED state: %d\n", led_state);
        gpio_write(led_pin, led_state);
        led_state = !led_state;
        delay_ms(500);
    }

    return 0;
}
