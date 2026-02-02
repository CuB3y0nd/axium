/**
 * @file oracle.c
 * @brief Implementation of the side-channel oracle specialization.
 */

#include <axium/sidechannel/oracle.h>
#include <limits.h>

int __attribute__((hot, flatten)) find_best_hit(const uint64_t *data,
                                                size_t len) {
  uint64_t min_time = UINT64_MAX;
  int best_index = -1;

  for (size_t i = 0; i < len; i++) {
    if (data[i] > 0 && data[i] < min_time) {
      min_time = data[i];
      best_index = (int)i;
    }
  }
  return best_index;
}

/**
 * @brief Internal query implementation for side-channel oracles.
 */
static int sidechannel_query_internal(oracle_t *base, size_t input) {
  sidechannel_oracle_t *o = (sidechannel_oracle_t *)base;

  /* 1. Trigger */
  o->ops.trigger(input, base->ctx);

  /* 2. Wait */
  if (!o->ops.wait(base->ctx)) {
    return -1;
  }

  /* 3. Analyze */
  if (o->ops.analyze) {
    return o->ops.analyze(o->data, o->data_len, base->ctx);
  }
  return find_best_hit(o->data, o->data_len);
}

void sidechannel_oracle_init(sidechannel_oracle_t *o, sidechannel_ops_t ops,
                             const uint64_t *data, size_t data_len, void *ctx) {
  if (!o)
    return;
  o->base.query = sidechannel_query_internal;
  o->base.ctx = ctx;
  o->ops = ops;
  o->data = data;
  o->data_len = data_len;
}
