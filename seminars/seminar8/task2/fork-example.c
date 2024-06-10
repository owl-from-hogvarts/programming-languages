/* fork-example-1.c */

#include "pipe-example.c"

#include <inttypes.h>
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
  void * shmem = create_shared_memory(INT_BUFFER_SIZE);
  int32_t * int_shared_buffer = shmem;

  for (size_t i = 0; i < INT_AMOUNT; i++) {
    int_shared_buffer[i] = i + 1;
  }

  int pipes[2][2];
  pipe(pipes[0]);
  pipe(pipes[1]);

  printf("Shared memory at: %p\n", shmem);
  int pid = fork();

  if (pid == 0) {
    // child process

    // Сохраним нужные дескпиторы конвейеров
    int to_parent_pipe = pipes[1][1];
    int from_parent_pipe = pipes[0][0];
    // И закроем ненужные
    close(pipes[1][0]);
    close(pipes[0][1]);

    while (true) {
      int64_t index = 0;
      int32_t new_value = 0;

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
        abort();
      }
      scanf("%" SCNd32, &new_value);

      int_shared_buffer[index] = new_value;

      struct message sync_message = simple_message(MSG_SYNC);
      send(to_parent_pipe, &sync_message);
    }

    close(to_parent_pipe);
    close(from_parent_pipe);
  } else {
    // parent process

    // Сохраним нужные дескпиторы конвейеров
    int from_child_pipe = pipes[1][0];
    int to_child_pipe = pipes[0][1];
    // И закроем ненужные
    close(pipes[1][1]);
    close(pipes[0][0]);

    printf("Child's pid is: %d\n", pid);

    struct message msg;
    while (true) {
      const int status_code = receive(from_child_pipe, &msg);
      if (status_code == 0) {
        break;
      }

      if (msg.header.type == MSG_SYNC) {
        printf("Data in buffer:\n");
        for (size_t i = 0; i < INT_AMOUNT; i++) {
          printf("%" PRIdPTR ": %" PRId32 "\n", i, int_shared_buffer[i]);
        }
      }
    }

    // Дождемся завершения ребенка
    waitpid(pid, NULL, 0);

    // Закроем дескпиторы
    close(from_child_pipe);
    close(to_child_pipe);

    return 0;
  }
}