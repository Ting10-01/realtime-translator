#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <thread>
#include <iostream>
extern "C" {
    #include "ring_buffer.h"
}

using namespace std;

void capture_audio(RingBuf *rb) {
    // Capture audio from the microphone and save it into a ring buffer
    
    // [TODO] Get the audio data from the microphone (use wav file for testing now)
    SF_INFO sfinfo;
    SNDFILE *infile = sf_open("./audio/wav/8842-302196-0000.wav", SFM_READ, &sfinfo);
    if (infile == NULL) {
        printf("Failed to open audio file\n");
        return;
    }

    float *buffer = (float *)malloc(sizeof(float) * sfinfo.frames * sfinfo.channels);
    int read_count = sf_readf_float(infile, buffer, BUFFER_SIZE - 1);
    sf_close(infile);
    if (read_count < 0) {
        printf("Failed to read audio data\n");
        return;
    }

    write_buf(rb, buffer, read_count);
    free(buffer);
}

void recognize_speech() { 
    // Use the Whisper API to recognize speech from the audio data
    // This function will be implemented in Python
}

void translate_speech() {
    // Translate the recognized speech into another language
    // This function will be implemented in Python
}

void display_translated_text() {
    // Display the translated text on the screen
    // This function will be implemented in C++
}

int main(int argc, char *argv[]) {
    RingBuf *rb = init_buf();
    if (rb == NULL) {
        printf("Failed to initialize ring buffer\n");
        return -1;
    }

    // The thread that handles audio input and save it into the ring buffer
    thread audio_thread(capture_audio, rb);
    
    // The thread that recognizes speech using the Whisper API
    thread recognition_thread(recognize_speech);

    // The thread that handles the translation (Python)
    thread translator_thread(translate_speech);
    
    // The thread that diplays the translated text (C++)
    thread display_thread(display_translated_text);

    // Wait for all threads to finish
    // The join() function returns when the thread execution has completed.
    audio_thread.join();
    recognition_thread.join();
    translator_thread.join();
    display_thread.join();

    cout << "Finish" << endl;

    delete rb;

    return 0;
}