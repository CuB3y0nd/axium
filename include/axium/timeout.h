#ifndef AXIUM_TIMEOUT_H
#define AXIUM_TIMEOUT_H

#define TIMEOUT_FOREVER ((double)-1.0)
#define TIMEOUT_DEFAULT ((double)-2.0)
#define TIMEOUT_MAXIMUM ((double)1048576.0)

/**
 * @brief Returns the current monotonic time in seconds.
 */
double timeout_now(void);

#endif // AXIUM_TIMEOUT_H
