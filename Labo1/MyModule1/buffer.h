#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>

// Macros ---------------------------------------------------------------------
#define BUFFER_NEW(x, y) \
    uint8_t x##_data_array[y]; \
    buffer_t x = { \
        .data = x##_data_array, \
        .count = 0, \
        .idx_in = 0, \
        .idx_out = 0, \
        .size = y \
    }

// Structures -----------------------------------------------------------------
typedef struct {
    volatile uint8_t *data;
    volatile uint32_t idx_in;
    volatile uint32_t idx_out;
    volatile uint32_t size;
    volatile uint32_t count;
} buffer_t;

// Public prototypes  ---------------------------------------------------------
void buffer_new(buffer_t *buffer, uint8_t *data, uint32_t size);

int32_t buffer_push(buffer_t *buffer, uint8_t data);

int32_t buffer_pull(buffer_t *buffer, uint8_t *ch);

int32_t buffer_count(buffer_t *buffer);

int32_t buffer_size(buffer_t *buffer);

void buffer_flush(buffer_t *buffer);

#endif
