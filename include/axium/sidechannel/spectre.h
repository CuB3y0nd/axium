/**
 * @file spectre.h
 * @brief Spectre transient execution attack primitives.
 */

#ifndef AXIUM_SPECTRE_H
#define AXIUM_SPECTRE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Variants of the Spectre vulnerability.
 */
typedef enum {
  SPECTRE_V1_BCB,     /**< Bounds Check Bypass (v1) */
  SPECTRE_V1_1_STORE, /**< TODO: Speculative Store Bypass (v1.1) */
  SPECTRE_V1_2_READ,  /**< TODO: Speculative Read-only Bypass (v1.2) */
  SPECTRE_V2_BTI      /**< TODO: Branch Target Injection (v2) */
} spectre_variant_t;

/**
 * @brief Callback to set the training or attack value.
 *
 * Use this if setting the value requires more than a simple memory write
 * (e.g., invoking a syscall).
 *
 * @param val The value to set (training or attack value).
 * @param ctx User-provided context.
 */
typedef void (*spec_setup_t)(size_t val, void *ctx);

/**
 * @brief Function pointer for the victim trigger callback.
 *
 * This callback is invoked by the attack engine to execute the victim code
 * path that contains the speculative vulnerability.
 *
 * @param ctx User-provided context passed through from the attack function.
 */
typedef void (*victim_trigger_t)(void *ctx);

/**
 * @brief Unified configuration for Spectre-style speculative attacks.
 *
 * This structure contains common synchronization parameters and a union
 * for variant-specific attack targets.
 */
typedef struct {
  spectre_variant_t variant; /**< Specific Spectre variant to trigger. */
  /* Variant-specific targets */
  union {
    /* Fields for V1 variants (PHT-based) */
    struct {
      void *index_addr;      /**< Address of the boundary/index variable. */
      size_t index_size;     /**< Size of the index variable (1, 2, 4, 8). */
      size_t training_val;   /**< Value that satisfies the branch condition. */
      size_t attack_val;     /**< Value that triggers speculative access. */
      spec_setup_t setup_fn; /**< Optional: Callback to set the value. */
    } v1;

    /* TODO: Fields for V2 variants (BTB-based) */
    struct {
      void *branch_source; /**< Instruction address to hijack. */
      void *branch_target; /**< Gadget address to speculatively jump to. */
    } v2;
  };

  /* Common synchronization and accumulation parameters */
  uint32_t ratio;      /**< Training rounds per attack round. */
  uint32_t trials;     /**< Total signal accumulation trials. */
  uint32_t sync_delay; /**< Cycles to wait after clflush of the index. */
  uint32_t post_delay; /**< Cycles to wait for victim to finish. */
} spectre_config_t;

/**
 * @brief Orchestrates a high-performance Spectre-V1 class attack.
 *
 * This function serves as the engine for all PHT (Pattern History Table) based
 * speculative execution attacks, including Bounds Check Bypass (v1),
 * Speculative Store Bypass (v1.1), and Read-only Bypass (v1.2).
 *
 * It automates the precise microarchitectural orchestration required to
 * trigger speculation by combining branch predictor training, memory
 * hierarchy stalling (via cache invalidation), and synchronization.
 *
 * ### V1-Class Orchestration Workflow:
 * 1. **Phase 1: Training** - Executes the victim @p ratio times with
 *    @p config->v1.training_val to bias the PHT towards a specific path.
 * 2. **Phase 2: Stalling** - Flushes @p config->v1.index_addr from the cache
 *    to force the CPU to stall while resolving the branch condition.
 * 3. **Phase 3: Speculation** - Triggers the victim with @p
 * config->v1.attack_val. The CPU speculatively executes the "Taken" path during
 * the stall.
 * 4. **Phase 4: Accumulation** - Repeats the cycle @p trials times to
 *    amplify the side-channel signal.
 *
 * @note This engine is optimized for timing determinism using `hot` and
 *       `flatten` attributes.
 *
 * @param config      Pointer to the Spectre configuration. The @p variant
 *                    field must be one of the V1 variants.
 * @param trigger_fn  Callback to invoke the victim's vulnerable code path.
 * @param trigger_ctx Context passed to @p trigger_fn.
 */
void spectre_v1(const spectre_config_t *config, victim_trigger_t trigger_fn,
                void *trigger_ctx);

#endif /* AXIUM_SPECTRE_H */
