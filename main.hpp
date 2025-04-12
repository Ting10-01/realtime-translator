#ifndef MAIN_HPP

#define MAIN_HPP

extern "C" {
    #include "ring_buffer.h"
}

#define SHM_NAME "/AUDIO_BUFFER"
#define MEMORY_SIZE (BUFFER_SIZE * sizeof(float))

RingBuf* init_rb();
std::pair<int, float*> init_shm();
void capture_audio(RingBuf* rb);
void write_shared_memory(RingBuf* rb, float* shm);
void clean_up(RingBuf* rb, int* shm_fd, float* shm);

#endif
