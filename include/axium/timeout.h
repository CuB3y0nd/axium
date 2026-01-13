/**
 * @file timeout.h
 * @brief Timeout management utilities.
 */

#ifndef AXIUM_TIMEOUT_H
#define AXIUM_TIMEOUT_H

/** @brief Constant representing infinite timeout. */
#define TIMEOUT_FOREVER ((double)-1.0)
/** @brief Constant representing default timeout. */
#define TIMEOUT_DEFAULT ((double)-2.0)
/** @brief Maximum allowed timeout value. */
#define TIMEOUT_MAXIMUM ((double)1048576.0)

/**
 * @brief Returns the current monotonic time in seconds.
 *
 * This uses CLOCK_MONOTONIC to provide a stable time reference for timeouts.
 *
 * @return Current time in seconds as a double.
 */
double timeout_now(void);

#endif // AXIUM_TIMEOUT_H
