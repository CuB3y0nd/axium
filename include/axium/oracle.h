/**
 * @file oracle.h
 * @brief Top-level generic Oracle abstraction for query-response models.
 *
 * This module provides a generic interface for oracles that can be queried
 * with an input to produce a result. This is commonly used in side-channel
 * attacks where an oracle represents a specialized primitive that leaks
 * information.
 */

#ifndef AXIUM_ORACLE_H
#define AXIUM_ORACLE_H

#include <stddef.h>
#include <sys/types.h>

struct oracle;

/**
 * @brief Function signature for a generic oracle query.
 *
 * @param self  Pointer to the oracle instance.
 * @param input The input value to query.
 * @return The oracle's response, or -1 on error.
 */
typedef int (*oracle_query_t)(struct oracle *self, size_t input);

/**
 * @brief Generic Oracle structure.
 *
 * Acts as a base class for specialized oracle implementations.
 */
typedef struct oracle {
  oracle_query_t query; /**< Query implementation. */
  void *ctx;            /**< Implementation-specific context. */
} oracle_t;

/**
 * @brief Performs a single query on the given oracle.
 *
 * @param o     The oracle to query.
 * @param input The input for the query.
 * @return The result of the query, or -1 on error.
 */
static inline int oracle_query(oracle_t *o, size_t input)
    __attribute__((warn_unused_result));

static inline int oracle_query(oracle_t *o, size_t input) {
  if (!o || !o->query) {
    return -1;
  }
  return o->query(o, input);
}

/**
 * @brief Scans (leaks) a sequence of data using an oracle.
 *
 * This utility iterates through inputs and stores the oracle results in a
 * buffer. It stops if the oracle returns -1 or if the terminator (if provided)
 * is reached.
 *
 * @param o          The oracle to use for scanning.
 * @param buf        The output buffer to store leaked results.
 * @param len        Maximum number of elements to leak.
 * @param terminator If non-negative, scanning stops when this value is leaked.
 * @return The number of elements successfully leaked, or -1 on early failure.
 */
ssize_t oracle_scan(oracle_t *o, char *buf, size_t len, int terminator)
    __attribute__((warn_unused_result));

#endif /* AXIUM_ORACLE_H */
