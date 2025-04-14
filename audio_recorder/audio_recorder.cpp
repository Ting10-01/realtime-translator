#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include "portaudio.h"
#include "audio_recorder.hpp"

using namespace std;

audio_recorder::audio_recorder() {
    rb = (RingBuf*)malloc(sizeof(RingBuf));
    if(!init_buf(rb, RING_BUFFER_SIZE))
        throw std::runtime_error("Failed to initialize ring buffer\n");
    
    PaError err = Pa_Initialize();
    if (err != paNoError)
        throw std::runtime_error("Failed to initialize PortAudio: " + std::string(Pa_GetErrorText(err)));
    err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, audio_to_buffer, rb);
    if (err != paNoError)
        throw std::runtime_error("Failed to open audio stream: " + std::string(Pa_GetErrorText(err)));
}

audio_recorder::~audio_recorder() {
    PaError err = Pa_StopStream(stream);
        cout << "Failed to stop audio stream: " + std::string(Pa_GetErrorText(err)) + "\n";
    err = Pa_CloseStream(stream);
    if (err != paNoError)
        cout << "Failed to close audio stream: " + std::string(Pa_GetErrorText(err)) + "\n";
    err = Pa_Terminate();
    if (err != paNoError)
        cout << "Failed to terminate PortAudio: " + std::string(Pa_GetErrorText(err)) + "\n";
    
    if (rb)
        free_buf(rb);
}

bool audio_recorder::start_recording() {
    PaError err = Pa_StartStream(stream);
    if (err != paNoError) {
        cout << "Failed to start audio stream: " + std::string(Pa_GetErrorText(err)) + "\n";
        return false;
    }
    return true;
}


bool audio_recorder::stop_recording() {
    PaError err = Pa_StopStream(stream);
    if (err != paNoError) {
        cout << "Failed to stop audio stream: " + std::string(Pa_GetErrorText(err)) + "\n";
        return false;
    }
    return true;
}

size_t audio_recorder::read_audio_data(float *data, size_t size) {
    return read_buf(rb, data, size);
}

int audio_recorder::audio_to_buffer(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    // if the input buffer is NULL, it means no audio data is available
    if (inputBuffer == NULL) return paContinue;

    write_buf((RingBuf*)userData, (float*)inputBuffer, framesPerBuffer);

    return paContinue;
}