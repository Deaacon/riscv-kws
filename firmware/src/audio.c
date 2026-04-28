#include <math.h>
#include "audio.h"
#include "driver/i2s_std.h"
#include "kiss_fftr.h"


#include "driver/i2s_std.h"
#include "driver/gpio.h"

i2s_chan_handle_t rx_handle;
i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(48000),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = GPIO_NUM_4,
        .ws = GPIO_NUM_5,
        .dout = I2S_GPIO_UNUSED,
        .din = GPIO_NUM_19,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
    },
};

void audio_init(int bck_pin, int ws_pin, int sd_pin) {
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    i2s_channel_init_std_mode(rx_handle, &std_cfg);
}

bool audio_is_segment_ready(void) {
}

int16_t audio_read_raw(void) {
    i2s_channel_enable(rx_handle);
    i2s_channel_read(rx_handle, desc_buf, bytes_to_read, bytes_read, ticks_to_wait);
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
