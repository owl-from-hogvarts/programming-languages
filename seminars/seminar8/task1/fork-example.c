/* fork-example-1.c */

#include <stdint.h>
#include <inttypes.h>
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
  void * shmem = create_shared_memory(INT_BUFFER_SIZE);
  int32_t * int_shared_buffer = shmem;

  for (size_t i = 0; i < INT_AMOUNT; i++) {
    int_shared_buffer[i] = i + 1;
  }

  printf("Shared memory at: %p\n", shmem);
  int pid = fork();

  if (pid == 0) {
    // child process
    size_t index = 0;
    int32_t new_value = 0;

    scanf("%" PRIdPTR , &index);
    scanf("%" SCNd32, &new_value);
    if (index >= INT_AMOUNT) {
      fprintf(stderr, "Too large index! Maximum allowed is: %zu" " Got: %zu\n", INT_AMOUNT - 1, index);
      abort();
    }
    int_shared_buffer[index] = new_value;
  } else {
    // parent process
    printf("Child's pid is: %d\n", pid);
    wait(NULL);
    for (size_t i = 0; i < INT_AMOUNT; i++) {
      printf("%" PRIdPTR ": %" PRId32 "\n", i, int_shared_buffer[i]);
    }
  }
}