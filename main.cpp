#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <thread>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include "main.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    RingBuf* rb = NULL;
    int shm_fd = -1;
    float* shm = NULL;
    
    try{
        rb = init_rb();
        tie(shm_fd, shm) = init_shm();
    }
    catch (const runtime_error &e) {
        cout << e.what() << endl;
        return -1;
    }

    // The thread that handles audio input and save it into the ring buffer
    // thread audio_reader(capture_audio, rb);

    // The thread that write the audio data from the ring buffer to shared memory
    // thread buffer_writer(write_shared_memory, rb, shm);

    capture_audio(rb);
    write_shared_memory(rb, shm);
    
    // Wait for all threads to finish
    // The join() function returns when the thread execution has completed.
    // audio_reader.join();
    // buffer_writer.join();

    clean_up(rb, &shm_fd, shm);
    
    cout << "Finished!" << endl;

    return 0;
}

RingBuf* init_rb() {
    RingBuf* rb = init_buf();
    if (rb == NULL)
        throw runtime_error("Failed to initialize ring buffer\n");
    return rb;
}

pair<int, float*> init_shm() {
    // Unlink the shared memory object if it already exists
    shm_unlink(SHM_NAME);

    // Create shared memory for transferring data between this process and the Python process
    // O_CREATE creates the shared memory object if it does not exist
    // O_RDWR opens the shared memory object for reading and writing
    // 0666 sets the permissions for the shared memory object (read and write for all users)
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0)
        throw runtime_error("create shared memory failed: " + string(strerror(errno)) + "\n");
    
    // The size of the shared memory object is set to twice the size of the ring buffer to implement the mirroring
    int status = ftruncate(shm_fd, MEMORY_SIZE);
    if (status < 0)
        throw runtime_error("ftruncate failed: " + string(strerror(errno)) + "\n");

    // Map the shared memory to the process's address space
    float* shm = (float*)mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED)
        throw runtime_error("map shared memory failed: " + string(strerror(errno)) + "\n");

    return {shm_fd, shm};
}

void capture_audio(RingBuf* rb) {
    // Capture audio from the microphone and save it into a ring buffer
    
    // [TODO] Get the audio data from the microphone (use wav file for testing now)
    SF_INFO sfinfo;
    SNDFILE *infile = sf_open("./audio/wav/8842-302196-0000.wav", SFM_READ, &sfinfo);

    float *buffer = (float *)malloc(sizeof(float) * sfinfo.frames * sfinfo.channels);
    int read_count = sf_readf_float(infile, buffer, BUFFER_SIZE - 1);
    sf_close(infile);
    if (read_count <= 0) {
        printf("Failed to read audio data\n");
        return;
    }

    write_buf(rb, buffer, read_count);
    
    free(buffer);
}

void write_shared_memory(RingBuf* rb, float* shm) {
    read_buf(rb, shm, BUFFER_SIZE);
}

void clean_up(RingBuf* rb, int* shm_fd, float* shm) {
    free(rb);
    munmap(shm, MEMORY_SIZE);
    shm_unlink(SHM_NAME);
    close(*shm_fd);
}