/**
 * @file oracle.h
 * @brief Top-level generic Oracle abstraction for query-response models.
 */

#ifndef AXIUM_ORACLE_H
#define AXIUM_ORACLE_H

#include <stddef.h>

struct oracle;

/**
 * @brief Function signature for a generic oracle query.
 * @param self Pointer to the oracle instance.
 * @param input The input value to query.
 * @return The oracle's response, -1 on error.
 */
typedef int (*oracle_query_t)(struct oracle *self, size_t input);

/**
 * @brief Generic Oracle structure.
 */
typedef struct oracle {
  oracle_query_t query; /**< Query implementation. */
  void *ctx;            /**< Implementation-specific context. */
} oracle_t;

/**
 * @brief Performs a query on the given oracle.
 */
static inline int query(oracle_t *o, size_t input) {
  if (!o || !o->query)
    return -1;
  return o->query(o, input);
}

#endif /* AXIUM_ORACLE_H */
