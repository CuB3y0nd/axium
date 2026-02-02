/**
 * @file timeout.h
 *
 * @brief Provides timeout management utilities.
 */

#ifndef AXIUM_TIMEOUT_H
#define AXIUM_TIMEOUT_H

#include <stdbool.h>
#include <unistd.h>

/** Constant representing infinite timeout. */
#define TIMEOUT_FOREVER ((double)-1.0)
/** Constant representing default timeout. */
#define TIMEOUT_DEFAULT ((double)-2.0)
/** Maximum allowed timeout value. */
#define TIMEOUT_MAXIMUM ((double)1048576.0)

/**
 * Returns the current monotonic time in seconds.
 *
 * This uses `CLOCK_MONOTONIC` to provide a stable time reference for timeouts.
 *
 * @return Current time in seconds.
 */
double timeout_now(void);

/**
 * @brief Waits for a condition to be met or until a timeout occurs.
 *
 * @param cond_expr   Condition expression.
 * @param timeout_sec Timeout in seconds.
 * @param sleep_usec  Sleep time in microseconds.
 * @return true if met, false on timeout.
 */
#define wait_until(cond_expr, timeout_sec, sleep_usec)                         \
  ({                                                                           \
    double __start = timeout_now();                                            \
    bool __res = false;                                                        \
    while (timeout_now() - __start < (double)(timeout_sec)) {                  \
      if (cond_expr) {                                                         \
        __res = true;                                                          \
        break;                                                                 \
      }                                                                        \
      if ((sleep_usec) > 0) {                                                  \
        usleep(sleep_usec);                                                    \
      }                                                                        \
    }                                                                          \
    __res;                                                                     \
  })

#endif /* AXIUM_TIMEOUT_H */
