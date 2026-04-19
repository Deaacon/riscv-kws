#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define AUDIO_SAMPLE_RATE  16000
#define AUDIO_BUFFER_SIZE  512
#define SPECTROGRAM_FRAMES 124  // При длине 16000 и шаге 128
#define FFT_SIZE           256  // Ближайшая степень двойки для кадра 255
#define FFT_BINS           129
#define SPECTROGRAM_SIZE   (SPECTROGRAM_FRAMES * FFT_BINS)

void audio_init(void);
int16_t audio_read_raw(void);
void audio_fill_buffer(int16_t *buffer, uint32_t len);
void audio_get_spectrogram(int16_t *samples, int16_t *output_spectrogram);

#endif
