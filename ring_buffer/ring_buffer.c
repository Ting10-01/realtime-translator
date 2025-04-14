#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "ring_buffer.h"

bool init_buf(RingBuf *rb, size_t buffer_size) {
    // Check if the size is a power of 2 to use mask for wrap around
    if (buffer_size < 1 || (buffer_size & (buffer_size - 1)) != 0) return false;

    rb->buffer_size = buffer_size;

    rb->index_mask = buffer_size - 1;
    rb->data_count_mask = (buffer_size << 1) - 1;
    rb->widx = 0;
    rb->ridx = 0;
    rb->buffer = (float*)malloc(buffer_size * sizeof(float));

    return true;
}

bool free_buf(RingBuf *rb) {
    if (rb == NULL) return false;
    free(rb->buffer);
    free(rb);
    
    return true;
}

size_t space_in_buf(RingBuf *rb) {
    return rb->buffer_size - data_in_buf(rb);
}

size_t data_in_buf(RingBuf *rb) {
    // rb->widx and rb->ridx are the index without wrap around
    // difference between widx and ridx is the number of elements in the buffer
    // & mask to ensure the difference is within the buffer size
    return ((rb->widx - rb->ridx) & rb->data_count_mask);
}

size_t write_buf(RingBuf *rb, float *data, size_t data_size) {
    size_t available_space = space_in_buf(rb);
    if (available_space < data_size)
        data_size = available_space;

    rb_index idx = (rb->widx & rb->index_mask);
    rb_index space_to_end = rb->buffer_size - idx;

    if (space_to_end >= data_size)
        memcpy(rb->buffer + idx, data, data_size * sizeof(float));
    else {
        // copy the first half of the data to the end of the buffer
        memcpy(rb->buffer + idx, data, space_to_end * sizeof(float));
        // copy the second half of the data to the beginning of the buffer
        memcpy(rb->buffer, data + space_to_end, (data_size - space_to_end) * sizeof(float));
    }

    // update the write index after writing the data
    __sync_synchronize();
    rb->widx += data_size;

    // return the number of data(float) written to the buffer
    return data_size;
}

size_t read_buf(RingBuf *rb, float *data, size_t data_size) {
    size_t available_data = data_in_buf(rb);
    if (available_data < data_size)
        data_size = available_data;

    rb_index idx = (rb->ridx & rb->index_mask);
    rb_index data_to_end = rb->buffer_size - idx;

    if (data_to_end >= data_size)
        memcpy(data, rb->buffer + idx, data_size * sizeof(float));
    else {
        memcpy(data, rb->buffer + idx, data_to_end * sizeof(float));
        memcpy(data + data_to_end, rb->buffer, (data_size - data_to_end) * sizeof(float));
    }

    __sync_synchronize();
    rb->ridx += data_size;

    return data_size;
}