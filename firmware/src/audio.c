#include <math.h>
#include "audio.h"
#include "hal/gdma_ll.h"
#include "hal/gpio_ll.h"
#include "hal/i2s_ll.h"
#include "soc/gpio_sig_map.h"
#include "soc/system_reg.h"
#include "rom/gpio.h"
#include "rom/esp_rom_gpio.h"
#include "kiss_fftr.h"

static int16_t adc_buffer[SAMPLES_1SEC] __attribute__((aligned(16)));

typedef struct {
    uint32_t dw0;
    uint32_t addr;
    uint32_t next;
} dma_desc_t;

static dma_desc_t dma_chain[10] __attribute__((aligned(16)));

void audio_init(int bck_pin, int ws_pin, int sd_pin) {
    // 1. Тактирование (через макросы из soc_struct или прямое управление)
    // В LL обычно нет функций включения питания модуля, используем системные макросы
    SET_PERI_REG_MASK(SYSTEM_PERIP_CLK_EN0_REG, SYSTEM_I2S0_CLK_EN | GDMA_CLK_EN);
    CLEAR_PERI_REG_MASK(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_I2S0_RST | SYSTEM_DMA_RST);

    // 2. GPIO Matrix через функции ROM (самый чистый способ для Makefile)
    esp_rom_gpio_connect_out_signal(bck_pin, I2SI_BCK_OUT_IDX, false, false);
    esp_rom_gpio_connect_out_signal(ws_pin, I2SI_WS_OUT_IDX, false, false);
    esp_rom_gpio_connect_in_signal(sd_pin, I2SI_SD_IN_IDX, false);

    // Включаем вход/выход через GPIO LL
    gpio_ll_input_enable(&GPIO, sd_pin);
    gpio_ll_output_enable(&GPIO, bck_pin);
    gpio_ll_output_enable(&GPIO, ws_pin);

    // 3. Настройка I2S через LL
    i2s_dev_t *i2s_hw = &I2S0;
    i2s_ll_rx_reset(i2s_hw);
    i2s_ll_rx_set_slave_mod(i2s_hw, false); // Master
    i2s_ll_rx_set_sample_bit(i2s_hw, 32, 16); // 32-bit slot, 16-bit data
    i2s_ll_rx_enable_mono_mode(i2s_hw, true);
    i2s_ll_rx_enable_msb_shift(i2s_hw, true); // Philips

    // Частота 16кГц: f = PLL / 156 / 64
    i2s_ll_rx_set_raw_clk_div(i2s_hw, 156, 0, 0, 1, 0);

    // 4. Настройка дескрипторов
    for (int i = 0; i < 10; i++) {
        dma_chain[i].dw0 = (1UL << 31) | ((i % 2 == 1) ? (1UL << 30) : 0) | 3200;
        dma_chain[i].addr = (uint32_t)&adc_buffer[i * 1600];
        dma_chain[i].next = (uint32_t)&dma_chain[(i + 1) % 10];
    }

    // 5. Настройка GDMA через LL
    gdma_dev_t *dma_hw = &GDMA;
    // Выбираем I2S как источник для входного канала 0
    gdma_ll_rx_connect_to_periph(dma_hw, 0, SOC_GDMA_TRIG_PERIPH_I2S0);
    // Устанавливаем адрес цепочки
    gdma_ll_rx_set_desc_addr(dma_hw, 0, (uint32_t)&dma_chain[0]);
    // Запускаем линк
    gdma_ll_rx_start(dma_hw, 0);

    // 6. Финальный запуск I2S
    i2s_ll_rx_enable_std(i2s_hw);
}

bool audio_is_segment_ready(void) {
    gdma_dev_t *dma_hw = &GDMA;
    // Проверяем статус прерывания через LL
    uint32_t status = gdma_ll_rx_get_interrupt_status(dma_hw, 0, true);
    if (status & GDMA_LL_EVENT_RX_SUC_EOF) {
        // Очищаем через LL
        gdma_ll_rx_clear_interrupt_status(dma_hw, 0, GDMA_LL_EVENT_RX_SUC_EOF);
        return true;
    }
    return false;
}

int16_t* audio_get_buffer(void) {
    return adc_buffer;
}

int16_t audio_read_raw(void) {
    return 0;
}

void audio_fill_buffer(int16_t *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        // buffer[i] = audio_read_raw();
        buffer[i] = i % 32768; // Заглушка для тестирования
        // for (volatile int d = 0; d < 2000; d++);
    }
}

static kiss_fftr_cfg cfg = NULL;
static uint8_t fft_buffer[2048] __attribute__((aligned(16)));

void audio_get_spectrogram(int16_t *samples, int16_t *output_spectrogram) {
    // Настройка KissFFT (нужно вызвать один раз, можно сделать статическим)
    printf("INFO: getting spectrogram\n");
    printf("Current cfg value: %p\n", (void*)cfg);

    static uint8_t first_run = 1;
    if (first_run) {
        cfg = NULL;
        first_run = 0;
    }

    if (cfg == NULL) {
        printf("INFO: allocating FFT config\n");
        size_t needed_size = sizeof(fft_buffer);
        printf("needed_size: %zu\n", needed_size);
        cfg = kiss_fftr_alloc(FFT_SIZE, 0, fft_buffer, &needed_size);
    }

    static kiss_fft_scalar timedata[FFT_SIZE];
    static kiss_fft_cpx freqdata[FFT_SIZE / 2 + 1];

    printf("CFG Address: %p\n", cfg);
    printf("INFO: performing computation\n");

    for (int frame = 0; frame < SPECTROGRAM_FRAMES; frame++) {
        // 1. Подготовка кадра (Windowing)
        printf("INFO: windowing frame\n");
        for (int i = 0; i < FFT_SIZE; i++) {
            int sample_idx = frame * 128 + i;
            if (sample_idx < SAMPLE_RATE) {
                timedata[i] = (kiss_fft_scalar)samples[sample_idx];
            } else {
                timedata[i] = 0;
            }
        }

        // 2. Выполнение FFT
        printf("INFO: computing fft\n");
        kiss_fftr(cfg, timedata, freqdata);

        // 3. Вычисление амплитуды (tf.abs)
        printf("INFO: computing abs\n");
        for (int i = 0; i < (FFT_SIZE / 2 + 1); i++) {
            kiss_fft_scalar re = freqdata[i].r;
            kiss_fft_scalar im = freqdata[i].i;
            kiss_fft_scalar magnitude = sqrtf(re * re + im * im);

            // Записываем в выходной массив (сплющенная спектрограмма)
            output_spectrogram[frame * (FFT_SIZE / 2 + 1) + i] = magnitude;
        }
    }
}
