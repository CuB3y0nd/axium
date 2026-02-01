/**
 * @file monitor.h
 *
 * High-level cache monitoring interfaces for side-channel analysis.
 */

#ifndef AXIUM_SIDECHANNEL_MONITOR_H
#define AXIUM_SIDECHANNEL_MONITOR_H

#include <axium/sidechannel/cache.h>

/**
 * @brief Callback function type for cache hit events.
 */
typedef void (*cache_hit_cb)(size_t index, uint64_t cycles, void *user_data);

/**
 * @brief Configuration for the cache watch.
 */
typedef struct {
  uint64_t threshold; /**< Cycle threshold for hit detection. */
  size_t count;       /**< Number of lines to monitor. */
  size_t stride;      /**< Distance between lines (e.g., PAGE_SIZE). */
  int wait_cycles;    /**< Micro-delay between Flush and Reload.
                       * Recommended range: 100-500 cycles.
                       * Shorter values increase sampling frequency but
                       * narrow the window for capturing victim activity.
                       * Longer values widen the capture window but increase
                       * system noise and decrease temporal resolution. */
} cache_watch_config;

/**
 * @brief Audits the environment to see if cache flushing is effective.
 *
 * @param target Pointer to a test memory location.
 * @param threshold The calibrated hit/miss threshold.
 * @return 0 if the environment is suitable, -1 if the audit fails.
 */
int cache_audit(const void *target, uint64_t threshold)
    __attribute__((warn_unused_result, nonnull(1)));

/**
 * @brief Watches a memory region for cache access events (Flush + Reload).
 *
 * This function is blocking and optimized for high-performance execution.
 *
 * @param base Base address of the shared memory region.
 * @param config Watch configuration.
 * @param callback Function to call when a hit is detected.
 * @param user_data User-defined data passed to the callback.
 */
void cache_watch(const void *base, const cache_watch_config *config,
                 cache_hit_cb callback, void *user_data)
    __attribute__((hot, flatten, nonnull(1, 2, 3)));

#endif /* AXIUM_SIDECHANNEL_MONITOR_H */
