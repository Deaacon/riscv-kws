#include "audio.h"
#include "system.h"

void audio_init(void) {
    // 1. Включаем тактирование АЦП (через системные регистры)
    // В Bare Metal это зависит от того, как настроен ваш startup (boot.c)

    // 2. Настройка разрешения (12 бит) и аттенюатора (для диапазона 0-2.5V)
    // Настраиваем таблицу паттернов для канала 0 (GPIO0)
    volatile uint32_t *patt_tab = (uint32_t *)APB_SARADC_SAR1_PATT_TAB1_REG;
    *patt_tab = (0 << 24) | (3 << 26); // Канал 0, аттенюатор 11dB

    // 3. Сброс и базовая настройка контроллера
    volatile uint32_t *ctrl = (uint32_t *)APB_SARADC_CTRL_REG;
    *ctrl |= (1 << 0); // Включить контроллер АЦП
}

uint16_t audio_read_raw(void) {
    volatile uint32_t *sample_reg = (uint32_t *)APB_SARADC_ONETIME_SAMPLE_REG;

    *sample_reg |= (1 << 31);

    while (!(*sample_reg & (1 << 30)));

    return (uint16_t)(*sample_reg & 0xFFF);
}

void audio_fill_buffer(uint16_t *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = audio_read_raw();
        for (volatile int d = 0; d < 2000; d++);
    }
}
