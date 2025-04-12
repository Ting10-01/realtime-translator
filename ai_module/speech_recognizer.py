import whisper
import numpy as np
import mmap
import posix_ipc

SHM_NAME = "/AUDIO_BUFFER"
MEMORY_SIZE = 1048576 * 4

def read_audio_from_shared_memory():
    shm = posix_ipc.SharedMemory(SHM_NAME)
    data = mmap.mmap(shm.fd, shm.size, mmap.MAP_SHARED, mmap.PROT_READ)
    shm.close_fd()
    
    return np.frombuffer(data, dtype=np.float32)


def audio2text():
    audio_data = read_audio_from_shared_memory()

    result = model.transcribe(audio_data)

    return(result["text"])


if __name__ == "__main__":
    model = whisper.load_model("base")
    print(audio2text())