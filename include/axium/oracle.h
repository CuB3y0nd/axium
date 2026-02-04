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

/**
 * @brief Performs a statistical query by running multiple rounds and voting.
 *
 * If @p votes is provided, the results are accumulated into it (the buffer
 * is NOT zeroed by this function). This allows for incremental sampling.
 *
 * @param o           The oracle instance.
 * @param input       The input value to query.
 * @param rounds      Number of sampling rounds to perform in this call.
 * @param threshold   Minimum number of votes required for a valid result.
 * @param votes       Optional buffer to store/accumulate votes.
 *                    Must be at least @p n_candidates long.
 * @param n_candidates Number of possible result values.
 * @return The winning value, or -1 if the threshold was not met.
 */
int oracle_query_stat(oracle_t *o, size_t input, int rounds, int threshold,
                      int *votes, size_t n_candidates)
    __attribute__((warn_unused_result));

/**
 * @brief Scans for a terminator using statistical voting and automated retries.
 *
 * This is the robust version of oracle_scan(). It is designed to leak data from
 * noisy oracles (like those found in side-channel attacks). For each index, it
 * performs multiple sampling rounds and uses a voting mechanism to determine
 * the most likely value.
 *
 * If the signal is too weak (no value meets the threshold), it will
 * automatically perform incremental retries, adding more samples until a
 * clear winner emerges or the retry limit is reached.
 *
 * @param o           The oracle instance to use for scanning.
 * @param buf         The output buffer to store leaked results.
 * @param len         Maximum number of elements to leak.
 * @param terminator  If non-negative, scanning stops when this value is leaked.
 * @param rounds      Number of sampling rounds to perform per attempt.
 * @param threshold   Minimum number of votes required to confirm a byte.
 * @param max_retries Maximum number of incremental retry attempts per byte.
 * @param votes       Optional buffer to store/accumulate votes for the current
 *                    byte. Must be at least @p n_candidates long.
 * @param n_candidates Number of possible result values (typically 256).
 * @return The number of elements successfully leaked, or -1 on early failure.
 */
ssize_t oracle_scan_stat(oracle_t *o, char *buf, size_t len, int terminator,
                         int rounds, int threshold, int max_retries, int *votes,
                         size_t n_candidates)
    __attribute__((warn_unused_result));

#endif /* AXIUM_ORACLE_H */
