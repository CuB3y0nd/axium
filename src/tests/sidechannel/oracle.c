#include <axium/axium.h>
#include <test_common.h>

typedef struct {
  uint64_t *timing_buffer;
  int target_hit;
} mock_ctx_t;

static void mock_trigger(size_t index, void *ctx) {
  (void)index;
  mock_ctx_t *m = (mock_ctx_t *)ctx;
  for (int i = 0; i < 256; i++)
    m->timing_buffer[i] = 200;
  m->timing_buffer[m->target_hit] = 50;
}

static bool mock_wait(void *ctx) {
  (void)ctx;
  return true;
}

static void test_layered_oracle(void) {
  uint64_t timings[256];
  mock_ctx_t ctx = {.timing_buffer = timings, .target_hit = 137};

  sidechannel_ops_t ops = {
      .trigger = mock_trigger, .wait = mock_wait, .analyze = NULL};

  sidechannel_oracle_t sc_oracle;
  sidechannel_oracle_init(&sc_oracle, ops, timings, 256, &ctx);

  /* Test generic query via the base oracle pointer */
  int result = query(&sc_oracle.base, 0);
  ASSERT_EQ(result, 137, "Generic query should return the mocked hit");
}

int main(void) {
  test_layered_oracle();
  log_success("Multi-layered oracle tests passed!");
  return 0;
}
