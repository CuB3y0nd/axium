#include <axium/utils/proc.h>
#include <limits.h>
#include <sys/wait.h>
#include <test_common.h>

int main(void) {
  ASSERT_TRUE(pid_exists(getpid()), "Current PID exists");

  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    usleep(100000); // 100ms
    exit(0);
  } else {
    bool found = wait_for_pid(pid, 500);
    ASSERT_TRUE(found, "Find child process within 500ms");

    bool timed_out = !wait_for_pid(INT_MAX, 50);
    ASSERT_TRUE(timed_out, "wait_for_pid(INT_MAX) timeouts");

    waitpid(pid, NULL, 0);
  }

  return 0;
}
