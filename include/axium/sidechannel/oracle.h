/**
 * @file oracle.h
 * @brief Side-channel specialization of the generic Oracle.
 *
 * This module specializes the generic oracle for side-channel measurement
 * workflows, typically involving a trigger, a wait for completion, and
 * an analysis of the results.
 */

#ifndef AXIUM_SIDECHANNEL_ORACLE_H
#define AXIUM_SIDECHANNEL_ORACLE_H

#include <axium/oracle.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Side-channel operations for the trigger-wait-analyze workflow.
 */
typedef struct {
  /**
   * @brief Triggers the victim or the event to be measured.
   * @param input The input value (e.g., an index or memory offset).
   * @param ctx   User-provided context.
   */
  void (*trigger)(size_t input, void *ctx);

  /**
   * @brief Waits for the measurement to complete or for a condition to be met.
   * @param ctx User-provided context.
   * @return true if the event occurred, false on timeout or error.
   */
  bool (*wait)(void *ctx);

  /**
   * @brief Analyzes the collected measurement data.
   * @param data Pointer to the timing or measurement data buffer.
   * @param len  Length of the data buffer.
   * @param ctx  User-provided context.
   * @return The resulting index or leaked value, -1 if no clear hit is found.
   */
  int (*analyze)(const uint64_t *data, size_t len, void *ctx);
} schan_ops_t;

/**
 * @brief Side-channel Oracle implementation.
 *
 * Implements the oracle_t interface using sc_ops_t.
 */
typedef struct {
  oracle_t base;        /**< Base generic oracle. */
  schan_ops_t ops;      /**< Side-channel specific operations. */
  const uint64_t *data; /**< Pointer to the measurement data buffer. */
  size_t data_len;      /**< Length of the measurement data buffer. */
} schan_oracle_t;

/**
 * @brief Initializes a side-channel oracle.
 *
 * @param o        Pointer to the side-channel oracle to initialize.
 * @param ops      The operations (trigger, wait, analyze).
 * @param data     The buffer where measurement results are stored.
 * @param data_len The size of the measurement data buffer.
 * @param ctx      User-provided context passed to all operations.
 */
void schan_oracle_init(schan_oracle_t *o, schan_ops_t ops, const uint64_t *data,
                       size_t data_len, void *ctx);

/**
 * @brief Standard analysis function to find the minimum timing (best hit).
 *
 * @param data Pointer to the timing data.
 * @param len  Length of the data.
 * @return Index of the minimum timing value, or -1 if all are zero.
 */
int find_best_hit(const uint64_t *data, size_t len)
    __attribute__((pure, nonnull(1), warn_unused_result));

#endif /* AXIUM_SIDECHANNEL_ORACLE_H */
