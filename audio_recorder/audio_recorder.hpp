#pragma once

extern "C" {
    #include "ring_buffer.h"
}

#include <iostream>
#include <portaudio.h>

#define SAMPLE_RATE (16000)  // 16000 frames per second
#define FRAMES_PER_BUFFER (1024) // 1024 frames in each callback
#define RING_BUFFER_SIZE (32768) // 2^15 frames in the ring buffer, about 2 seconds of audio data (16000 * 2 = 32000 frames)

class audio_recorder {
public:
    audio_recorder();
    ~audio_recorder();
    size_t read_audio_data(float *data, size_t size);
    bool start_recording();
    bool stop_recording();
private:
    // static to avoid it being called from outside the class
    static int audio_to_buffer(
        const void *inputBuffer, // the audio data from the microphone
        void *outputBuffer, // not used
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, // not used
        PaStreamCallbackFlags statusFlags, // not used
        void *userData // the ring buffer to store the audio data
    );
    
    PaStream *stream;
    RingBuf *rb;
};