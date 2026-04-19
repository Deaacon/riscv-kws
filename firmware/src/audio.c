#include <math.h>
#include "kiss_fftr.h"
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

int16_t audio_read_raw(void) {
    volatile uint32_t *sample_reg = (uint32_t *)APB_SARADC_ONETIME_SAMPLE_REG;

    // *sample_reg |= (1 << 31);

    // while (!(*sample_reg & (1 << 30)));

    return (int16_t)(*sample_reg & 0xFFF);
}

void audio_fill_buffer(int16_t *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = audio_read_raw();
        // for (volatile int d = 0; d < 2000; d++);
    }
}

static uint8_t fft_buffer[2048] __attribute__((aligned(16)));
static kiss_fftr_cfg cfg;

void audio_get_spectrogram(int16_t *samples, int16_t *output_spectrogram) {
    // Настройка KissFFT (нужно вызвать один раз, можно сделать статическим)
    printf("INFO: getting spectrogram\n");
    static kiss_fftr_cfg cfg = NULL;
    if (cfg == NULL) {
        size_t needed_size = sizeof(fft_buffer);
        cfg = kiss_fftr_alloc(FFT_SIZE, 0, fft_buffer, &needed_size);
    }

    static kiss_fft_scalar timedata[FFT_SIZE];
    static kiss_fft_cpx freqdata[FFT_SIZE / 2 + 1];

    if (cfg == NULL) { printf("ALARM: CFG IS NULL"); }
    printf("CFG Address: %p\n", cfg);
    printf("INFO: performing computation\n");
    for (int frame = 0; frame < SPECTROGRAM_FRAMES; frame++) {
        // 1. Подготовка кадра (Windowing)
        printf("INFO: windowing frame\n");
        for (int i = 0; i < FFT_SIZE; i++) {
            int sample_idx = frame * 128 + i;
            if (i < 255 && sample_idx < 16000) {
                // В Python версии нет окна Хэмминга, просто копируем
                timedata[i] = (float)samples[sample_idx];
            } else {
                timedata[i] = 0.0f; // Padding
            }
        }

        // 2. Выполнение FFT
        printf("INFO: computing fft\n");
        kiss_fftr(cfg, timedata, freqdata);

        // 3. Вычисление амплитуды (tf.abs)
        printf("INFO: computing abs\n");
        for (int i = 0; i < (FFT_SIZE / 2 + 1); i++) {
            float re = freqdata[i].r;
            float im = freqdata[i].i;
            float magnitude = sqrtf(re * re + im * im);

            // Записываем в выходной массив (сплющенная спектрограмма)
            output_spectrogram[frame * (FFT_SIZE / 2 + 1) + i] = magnitude;
        }
    }
}
