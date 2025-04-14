#pragma once

#define MAIN_HPP

#include "audio_recorder/audio_recorder.hpp"

#define SHM_NAME "/AUDIO_BUFFER"
#define MEMORY_SIZE (RING_BUFFER_SIZE * 20)

std::pair<int, float*> init_shm();
void clean_up(int* shm_fd, float* shm, float* data);