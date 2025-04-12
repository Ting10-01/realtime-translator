#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "ring_buffer.h"

RingBuf* init_buf() {
    RingBuf *rb = (RingBuf*)malloc(sizeof(RingBuf));
    if (rb == NULL)
        return NULL;
    
    rb->wptr = rb->buffer;
    rb->rptr = rb->buffer;

    pthread_mutex_init(&rb->mutex, NULL);

    return rb;
}

void write_buf(RingBuf *rb, float *data, int size) {
    pthread_mutex_lock(&rb->mutex);

    // Wait until there is enough space in the buffer
    while (size > space_in_buf(rb))
        // Sleep for 1ms to avoid busy waiting
        usleep(1000); 

    for (int i = 0; i < size; i++) {
        *rb->wptr = data[i];
        rb->wptr++;
        if (rb->wptr >= rb->buffer + BUFFER_SIZE)
            rb->wptr = rb->buffer;
    }

    pthread_mutex_unlock(&rb->mutex);
}

int read_buf(RingBuf *rb, float *data, int size) {
    pthread_mutex_lock(&rb->mutex);

    int available_data = data_in_buf(rb);
    if (available_data < size)
        size = available_data;

    for (int i = 0; i < size; i++) {
        data[i] = *rb->rptr;
        rb->rptr++;
        if (rb->rptr >= rb->buffer + BUFFER_SIZE)
            rb->rptr = rb->buffer;
    }

    pthread_mutex_unlock(&rb->mutex);

    return size;
}

int data_in_buf(RingBuf *rb) {
    return BUFFER_SIZE - space_in_buf(rb) - 1;
}

// Though the buffer size is definded as X, the actual size is X-1 to differentiate the empty and full state
int space_in_buf(RingBuf *rb) {
    // the type of wptr and rptr are "float *", so the difference is the number of elements
    // if the type of wptr and rptr are "size_t", the difference is the number of bytes so we need to divide it by sizeof(float)
    if (rb->wptr > rb->rptr)
        return BUFFER_SIZE - (rb->wptr - rb->rptr) - 1;
    
    if (rb->wptr < rb->rptr)
        return rb->rptr - rb->wptr - 1;
    
    // The buffer is empty
    return BUFFER_SIZE - 1;
}