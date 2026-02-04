/**
 * @file cpu.h
 *
 * Provides CPU affinity utilities for side-channel attacks.
 */

#ifndef AXIUM_SIDECHANNEL_CPU_H
#define AXIUM_SIDECHANNEL_CPU_H

#include <sched.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef _CPU_INLINE
#define _CPU_INLINE static inline __attribute__((always_inline))
#endif

/**
 * Sets the CPU affinity of a specific process.
 *
 * @param pid The process ID. 0 refers to the calling process.
 * @param cpu_core The CPU core index to pin to.
 */
void set_pid_cpu_affinity(pid_t pid, int cpu_core);

/**
 * Sets the CPU affinity of the current process/thread.
 *
 * @param cpu_core The CPU core index to pin to.
 */
_CPU_INLINE void set_cpu_affinity(int cpu_core) {
  set_pid_cpu_affinity(0, cpu_core);
}

#undef _CPU_INLINE

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
      void *index_addr;    /**< Address of the boundary/index variable. */
      size_t index_size;   /**< Size of the index variable (1, 2, 4, 8). */
      size_t training_val; /**< Value that satisfies the branch condition. */
      size_t attack_val;   /**< Value that triggers speculative access. */
    } v1;

    /* TODO: Fields for V2 variants (BTB-based) */
    struct {
      void *branch_source; /**< Instruction address to hijack. */
      void *branch_target; /**< Gadget address to speculatively jump to. */
    } v2;
  };

  /* Common synchronization and accumulation parameters */
  uint32_t ratio;      /**< Training rounds per attack round (e.g., 4:1). */
  uint32_t trials;     /**< Total signal accumulation trials. */
  uint32_t sync_delay; /**< Cycles to wait after clflush of the index. */
  uint32_t post_delay; /**< Cycles to wait for victim to finish. */
} spectre_config_t;

/**
 * @brief Busy-wait for a specific number of iterations using the 'pause'
 * instruction.
 */
static inline __attribute__((always_inline)) void delay_cycles(size_t cycles) {
  for (size_t i = 0; i < cycles; i++) {
    __asm__ __volatile__("pause" ::: "memory");
  }
}

#endif /* AXIUM_CPU_H */
