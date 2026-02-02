/**
 * @file oracle.h
 * @brief Side-channel specialization of the generic Oracle.
 */

#ifndef AXIUM_SIDECHANNEL_ORACLE_H
#define AXIUM_SIDECHANNEL_ORACLE_H

#include <axium/oracle.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Operations for side-channel oracles.
 */
typedef struct {
  /** Triggers the victim. Context is passed from sidechannel_oracle_t. */
  void (*trigger)(size_t input, void *ctx);

  /** Synchronizes with victim. */
  bool (*wait)(void *ctx);

  /** Analyzes the result buffer. If NULL, defaults to find_best_hit. */
  int (*analyze)(const uint64_t *data, size_t len, void *ctx);
} sidechannel_ops_t;

/**
 * @brief Side-channel Oracle implementation.
 *
 * Specializes oracle_t for trigger-wait-analyze workflows.
 */
typedef struct {
  oracle_t base;         /**< Base generic oracle. */
  sidechannel_ops_t ops; /**< Side-channel specific operations. */
  const uint64_t *data;  /**< Measurement data buffer. */
  size_t data_len;       /**< Length of the data buffer. */
} sidechannel_oracle_t;

/**
 * @brief Initializes a side-channel oracle.
 */
void sidechannel_oracle_init(sidechannel_oracle_t *o, sidechannel_ops_t ops,
                             const uint64_t *data, size_t data_len, void *ctx);

/**
 * @brief Standard analysis function to find the minimum timing.
 */
int find_best_hit(const uint64_t *data, size_t len)
    __attribute__((pure, nonnull(1), warn_unused_result));

#endif /* AXIUM_SIDECHANNEL_ORACLE_H */
