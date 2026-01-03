#ifndef AXIUM_PROC_H
#define AXIUM_PROC_H

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>

/**
 * @brief Checks if a process exists.
 *
 * Uses kill(pid, 0) which is the most efficient detection method.
 *
 * @param pid The process ID to check.
 * @return true if the process exists (even if owned by another user), false
 * otherwise.
 */
__attribute__((warn_unused_result)) static inline bool pid_exists(pid_t pid) {
  if (pid <= 0) {
    return false;
  }
  if (kill(pid, 0) == 0) {
    return true;
  }
  return errno == EPERM;
}

/**
 * @brief Waits for a given PID to become visible.
 *
 * @param pid The process ID to wait for.
 * @param timeout Maximum time to wait in milliseconds. 0 means infinite.
 * @return true if the process became visible, false on timeout.
 */
__attribute__((warn_unused_result)) bool wait_for_pid(pid_t pid, long timeout);

#endif // AXIUM_PROC_H
