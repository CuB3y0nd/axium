#include <axium/utils/proc.h>
#include <limits.h>
#include <sys/wait.h>
#include <test_common.h>

void test_pid_exists_self(void) {
  printf("--- Testing pid_exists(getpid()) ---\n");
  ASSERT_TRUE(pid_exists(getpid()), "Current PID exists");
}

void test_wait_for_pid_child(void) {
  printf("--- Testing wait_for_pid with child ---\n");
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    usleep(50000); // 50ms
    exit(0);
  } else {
    bool found = wait_for_pid(pid, 500);
    ASSERT_TRUE(found, "Find child process within timeout");
    waitpid(pid, NULL, 0);
  }
}

void test_wait_for_pid_timeout(void) {
  printf("--- Testing wait_for_pid timeout ---\n");
  // Assuming INT_MAX is not a running PID
  bool timed_out = !wait_for_pid(INT_MAX, 50);
  ASSERT_TRUE(timed_out, "wait_for_pid(INT_MAX) times out as expected");
}

int main(void) {
  test_pid_exists_self();
  test_wait_for_pid_child();
  test_wait_for_pid_timeout();
  return 0;
}
