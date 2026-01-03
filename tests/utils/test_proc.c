#include <assert.h>
#include <axium/utils/proc.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void) {
  printf("Running test_proc.c\n");

  printf("pid_exists(getpid()) result: %s\n",
         pid_exists(getpid()) ? "SUCCESS" : "FAILURE");

  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // Just stay alive long enough for the parent to check
    usleep(100000); // 100ms
    exit(0);
  } else {
    bool found = wait_for_pid(pid, 500);
    printf("wait_for_pid(child) result: %s\n", found ? "SUCCESS" : "FAILURE");
    assert(found == true);

    bool timed_out = !wait_for_pid(INT_MAX, 50);
    printf("wait_for_pid(INT_MAX) timeout check: %s\n",
           timed_out ? "PASSED" : "FAILED");
    assert(timed_out == true);

    waitpid(pid, NULL, 0);
  }

  return 0;
}
