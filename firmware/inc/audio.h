#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define AUDIO_SAMPLE_RATE  16000
#define AUDIO_BUFFER_SIZE  512

void audio_init(void);
uint16_t audio_read_raw(void);
void audio_fill_buffer(uint16_t *buffer, uint32_t len);

#endif
