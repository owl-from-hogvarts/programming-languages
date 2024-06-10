/* fork-example-1.c */

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <inttypes.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

void * create_shared_memory(size_t size) {
  return mmap(
      NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

const uint64_t INT_AMOUNT = 10;
const size_t INT_BUFFER_SIZE = sizeof(int32_t) * INT_AMOUNT;

int main() {
  sem_t * const write_allowed_semaphor
      = create_shared_memory(sizeof(sem_t) * 2);
  sem_t * const read_allowed_semaphor = write_allowed_semaphor + 1;
  sem_init(write_allowed_semaphor, true, 1);
  sem_init(read_allowed_semaphor, true, 1);
  int32_t * const int_shared_buffer = create_shared_memory(INT_BUFFER_SIZE);

  for (size_t i = 0; i < INT_AMOUNT; i++) {
    int_shared_buffer[i] = i + 1;
  }

  printf("Shared memory at: %p\n", int_shared_buffer);
  int pid = fork();

  if (pid == 0) {
    // child process

    while (true) {
      int64_t index = 0;
      int32_t new_value = 0;

      sem_wait(write_allowed_semaphor);
      scanf("%" PRId64, &index);
      if (index < 0) {
        puts("exiting");
        break;
      }
      if (index >= INT_AMOUNT) {
        fprintf(stderr,
                "Too large index! Maximum allowed is: %zu"
                " Got: %zu\n",
                INT_AMOUNT - 1,
                index);
        break;
      }
      scanf("%" SCNd32, &new_value);

      int_shared_buffer[index] = new_value;
      sem_post(read_allowed_semaphor);
    }

    abort();

  } else {
    // parent process

    printf("Child's pid is: %d\n", pid);

    while (true) {
      while (sem_trywait(read_allowed_semaphor) == -1 && errno == EAGAIN) {
        if ((waitpid(pid, NULL, WNOHANG) == pid)) {
          goto exit;
        }
      }
      printf("Data in buffer:\n");
      for (size_t i = 0; i < INT_AMOUNT; i++) {
        printf("%" PRIdPTR ": %" PRId32 "\n", i, int_shared_buffer[i]);
      }
      sem_post(write_allowed_semaphor);
    }
    exit: 
    return 0;
  }
}