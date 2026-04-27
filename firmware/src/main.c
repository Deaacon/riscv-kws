#include <system.h>
#include "audio.h"
#include "model_data.h"
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/all_ops_resolver.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"

#define SAMPLES 16000

static int led_pin = 8;
static int led_state = 0;

int errno;
int *__errno(void) { return &errno; }

int16_t raw_buffer[SAMPLES];
int16_t feature_buffer[SPECTROGRAM_SIZE];

int main(void) {
    wdt_disable();

    printf("\nHello World!\n");
    gpio_output(led_pin);

    // bck_pin = 5 (физически на SCK микрофона)
    // ws_pin = 4  (физически на WS микрофона)
    // sd_pin = 6  (данные)
    audio_init(5, 4, 6);

    printf("Audio system initialized. Starting record...\n");

    while (1) {
        if (audio_is_segment_ready()) {
            // Получаем указатель на начало буфера
            int16_t* buffer = audio_get_buffer();

            // Обрабатываем данные (например, считаем среднюю громкость сегмента)
            long sum = 0;
            for (int i = 0; i < SAMPLES_200MS; i++) {
                // Берем модуль значения (амплитуду)
                sum += (buffer[i] < 0) ? -buffer[i] : buffer[i];
            }

            int avg_amplitude = sum / SAMPLES_200MS;
            printf("Segment ready! Avg Amplitude: %d\n", avg_amplitude);
        }

        // printf("Filling buffer\n");
        // audio_fill_buffer(raw_buffer, SAMPLES);

        // printf("Getting spectrogram\n");
        // audio_get_spectrogram(raw_buffer, feature_buffer);

        // Запуск нейросети
        // copy_to_tensor_arena(feature_buffer);
        // TfLiteStatus invoke_status = interpreter->Invoke();

        // Анализ результата
        // process_output();

        printf("LED state: %d\n", led_state);
        gpio_write(led_pin, led_state);
        led_state = !led_state;
        delay_ms(500);
    }

    return 0;
}
