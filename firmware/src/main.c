#include <system.h>
#include "model_data.h"
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/all_ops_resolver.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"

static int led_pin = 8;
static int led_state = 0;

int main(void) {
    wdt_disable();

    printf("Hello World!\n");
    gpio_output(led_pin);

    for (;;) {
        // if is_audio_buffer_ready() {
        //     // Предобработка
        //     generate_spectrogram(audio_raw_data, feature_buffer);

        //     // Запуск нейросети
        //     copy_to_tensor_arena(feature_buffer);
        //     TfLiteStatus invoke_status = interpreter->Invoke();

        //     // Анализ результата
        //     process_output();
        // }
        printf("LED state: %d\n", led_state);
        gpio_write(led_pin, led_state);
        led_state = !led_state;
        delay_ms(500);
    }

    return 0;
}
