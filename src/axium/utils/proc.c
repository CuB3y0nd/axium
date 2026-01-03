#include <axium/utils/proc.h>
#include <time.h>

bool wait_for_pid(pid_t pid, long timeout_ms) {
  struct timespec start, now;

  if (timeout_ms > 0) {
    clock_gettime(CLOCK_MONOTONIC, &start);
  }

  while (true) {
    if (pid_exists(pid)) {
      return true;
    }

    if (timeout_ms > 0) {
      clock_gettime(CLOCK_MONOTONIC, &now);
      long elapsed = (now.tv_sec - start.tv_sec) * 1000 +
                     (now.tv_nsec - start.tv_nsec) / 1000000;
      if (elapsed >= timeout_ms) {
        break;
      }
    }

    struct timespec req = {0, 1000000}; // 1ms
    nanosleep(&req, NULL);
  }

  return false;
}
