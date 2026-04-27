#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

#define SAMPLE_RATE    16000
#define SEGMENT_MS     200
#define TOTAL_MS       1000
#define SAMPLES_1SEC   (SAMPLE_RATE)
#define SAMPLES_200MS  (SAMPLE_RATE * SEGMENT_MS / 1000)

void audio_init(int bck_pin, int ws_pin, int sd_pin);
bool audio_is_segment_ready(void);
int16_t* audio_get_buffer(void);

#define SPECTROGRAM_FRAMES 124  // При длине 16000 и шаге 128
#define FFT_SIZE           256  // Ближайшая степень двойки для кадра 255
#define FFT_BINS           129
#define SPECTROGRAM_SIZE   (SPECTROGRAM_FRAMES * FFT_BINS)

int16_t audio_read_raw(void);
void audio_fill_buffer(int16_t *buffer, uint32_t len);
void audio_get_spectrogram(int16_t *samples, int16_t *output_spectrogram);

#endif
