/**
 * @file timeout.h
 *
 * Provides timeout management utilities.
 */

#ifndef AXIUM_TIMEOUT_H
#define AXIUM_TIMEOUT_H

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

#endif // AXIUM_TIMEOUT_H