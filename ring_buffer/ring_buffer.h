#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
// use typedef to enhance the readability
typedef uint32_t rb_index;

typedef struct _RingBuf{
    size_t buffer_size; // the size of the buffer in float
    
    rb_index index_mask; // wrap around the writer and reader index, which is equal to (idx % buffer_size)
    uint32_t data_count_mask; // mask to get the number of data in the buffer
    
    volatile rb_index widx; // idx is always incremented by 1 and wrap around if it reaches the buffer size
    volatile rb_index ridx; // use volatile to prevent the compiler from optimizing the code and to ensure the value is always read from memory
    
    float *buffer;
} RingBuf;

/*
@param rb Pointer to the ring buffer
@param buffer_size The size of the ring buffer
@return True if the buffer is initialized successfully, false otherwise
*/
bool init_buf(RingBuf *rb, size_t buffer_size);
/*
@param rb Pointer to the ring buffer
@return True if the buffer is destroyed successfully, false otherwise
*/
bool free_buf(RingBuf *rb);

/*
@param rb Pointer to the ring buffer
@return The available float space in the buffer
*/
size_t space_in_buf(RingBuf *rb);

/*
@param rb Pointer to the ring buffer
@return The number of float datas in the buffer
*/
size_t data_in_buf(RingBuf *rb);

/*
@param rb Pointer to the ring buffer
@param data Pointer to the data to be written
@param size The size of the data to be written in float
@return The number of float datas written to the buffer
*/
size_t write_buf(RingBuf *rb, float *data, size_t data_size);
/*
@param rb Pointer to the ring buffer
@param data Pointer to the data to store the data read from the buffer
@param size The size of the data to be read in float
@return The number of float datas read from the buffer
*/
size_t read_buf(RingBuf *rb, float *data, size_t data_size);

#ifdef __cplusplus
}
#endif