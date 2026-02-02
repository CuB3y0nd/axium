/**
 * @file oracle.c
 *
 * @brief Implementation of the side-channel oracle specialization.
 */

#include <axium/sidechannel/oracle.h>
#include <limits.h>

int __attribute__((hot, flatten)) find_best_hit(const uint64_t *data,
                                                size_t len) {
  uint64_t min_time = UINT64_MAX;
  int best_index = -1;

  for (size_t i = 0; i < len; i++) {
    uint64_t t = data[i];
    /* Ignore zero as it usually means no measurement or invalid */
    if (__builtin_expect(t > 0, 1)) {
      if (t < min_time) {
        min_time = t;
        best_index = (int)i;
      }
    }
  }
  return best_index;
}

/**
 * @brief Internal query implementation for side-channel oracles.
 *
 * This function follows the trigger-wait-analyze sequence.
 */
static int schan_query_internal(oracle_t *base, size_t input) {
  schan_oracle_t *o = (schan_oracle_t *)base;

  /* 1. Trigger the event */
  o->ops.trigger(input, base->ctx);

  /* 2. Wait for completion */
  if (o->ops.wait && !o->ops.wait(base->ctx)) {
    return -1;
  }

  /* 3. Analyze the results */
  if (o->ops.analyze) {
    return o->ops.analyze(o->data, o->data_len, base->ctx);
  }
  return find_best_hit(o->data, o->data_len);
}

void schan_oracle_init(schan_oracle_t *o, schan_ops_t ops, const uint64_t *data,
                       size_t data_len, void *ctx) {
  if (!o || !data) {
    return;
  }

  o->base.query = schan_query_internal;
  o->base.ctx = ctx;
  o->ops = ops;
  o->data = data;
  o->data_len = data_len;
}
