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
    int shm_fd = -1;
    float* shm = NULL;

    // Initialize the ring buffer and port audio
    audio_recorder ar;
    float* data = NULL;
    
    try {
        // Initialize the shared memory
        tie(shm_fd, shm) = init_shm();
        // Initialize the data read from the ring buffer and write to the shared memory
        //data = (float*)malloc(sizeof(float) * RING_BUFFER_SIZE);
        // Start the audio recording
        ar.start_recording();
    }
    catch (const runtime_error &e) {
        std::cout << e.what() << endl;
        clean_up(&shm_fd, shm, data);
        return -1;
    }

    int time = 0; // temperary record time
    int total = 0; // total number of frames read from the ring buffer
    while (time < 10) {
        // wait for the audio data to be available
        std::this_thread::sleep_for(std::chrono::seconds(1));
        size_t data_size = ar.read_audio_data(shm + total, RING_BUFFER_SIZE);
        total += data_size;
        cout << "Read " << data_size << " frames from the ring buffer" << endl; 
        time ++; // simulate the recording time
    }

    ar.stop_recording();

    SF_INFO sfinfo;
    sfinfo.samplerate = SAMPLE_RATE;
    sfinfo.frames = 0;
    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SNDFILE *outfile = sf_open("record.wav", SFM_WRITE, &sfinfo);
    sf_write_float(outfile, shm, total);
    sf_close(outfile);

    clean_up(&shm_fd, shm, data);
    
    cout << "Finished!" << endl;

    return 0;
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

void clean_up(int* shm_fd, float* shm, float* data) {
    munmap(shm, MEMORY_SIZE * sizeof(float));
    shm_unlink(SHM_NAME);
    close(*shm_fd);

    free(data);
    data = NULL;
}