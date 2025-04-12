#ifdef __cplusplus
extern "C" {
#endif

// If RING_BUFFER_H is not defined, define it to prevent multiple inclusions
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define BUFFER_SIZE 1048576

#include <stddef.h>
#include <pthread.h>

// [TODO] Consider to make the buffer a generic type
typedef struct _RingBuf{
    float *wptr;
    float *rptr;
    float buffer[BUFFER_SIZE];
    // [TODO] Consider to support OS which does not support pthread
    pthread_mutex_t mutex;
} RingBuf;

RingBuf* init_buf();

void write_buf(RingBuf *rb, float *data, int size);
int read_buf(RingBuf *rb, float *data, int size);

int data_in_buf(RingBuf *rb);
int space_in_buf(RingBuf *rb);

#endif

#ifdef __cplusplus
}
#endif