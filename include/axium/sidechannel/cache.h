/**
 * @file cache.h
 *
 * Provides high-performance primitives for cache side-channel exploitation.
 *
 * This header includes specialized macros and inlined assembly wrappers for
 * cache manipulation, memory fencing, and high-resolution execution timing.
 */

#ifndef AXIUM_CACHE_H
#define AXIUM_CACHE_H

#include <stdint.h>

/**
 * @brief Shuffles an index using a Linear Congruential Mapping to defeat
 * hardware prefetchers.
 *
 * Formula: (i * 167 + 13) & mask.
 *
 * @param i The original sequential index.
 * @param mask The bitwise mask. MUST be (ARRAY_SIZE - 1), where ARRAY_SIZE is a
 * power of 2.
 */
#define MIXED_IDX(i, mask) (((i) * 167 + 13) & (mask))

/**
 * @brief Internal attribute for timing-critical inlining.
 */
#define _CACHE_INLINE static inline __attribute__((always_inline))

/**
 * @brief Reads the Time Stamp Counter (TSC).
 *
 * This is a raw, non-serializing read of the TSC.
 *
 * @return 64-bit tick count.
 */
_CACHE_INLINE uint64_t rdtsc(void) {
  uint32_t lo, hi;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi) : : "memory");
  return ((uint64_t)hi << 32) | lo;
}

/**
 * @brief Reads the Time Stamp Counter (TSC) and the Processor ID.
 *
 * This instruction is partially serializing: it waits until all previous
 * instructions have executed before reading the counter.
 *
 * @return 64-bit tick count.
 */
_CACHE_INLINE uint64_t rdtscp(void) {
  uint32_t lo, hi;
  __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi) : : "rcx", "memory");
  return ((uint64_t)hi << 32) | lo;
}

/**
 * @brief Full memory barrier.
 *
 * Ensures that all load and store instructions that precede the fence in
 * program order are globally visible before any instructions that follow.
 */
_CACHE_INLINE void mfence(void) { __asm__ __volatile__("mfence" ::: "memory"); }

/**
 * @brief Load memory barrier.
 *
 * Serializes load operations and prevents speculative execution of subsequent
 * instructions.
 */
_CACHE_INLINE void lfence(void) { __asm__ __volatile__("lfence" ::: "memory"); }

/**
 * @brief Store memory barrier.
 *
 * Ensures that all store operations that precede the fence are globally visible
 * before any subsequent stores.
 */
_CACHE_INLINE void sfence(void) { __asm__ __volatile__("sfence" ::: "memory"); }

/* clang-format off */
/**
 * @brief Serializing CPUID instruction.
 *
 * Forces the CPU to complete all preceding instructions before continuing.
 * Used as a strong barrier for side-channel timing.
 */
_CACHE_INLINE void cpuid(void) {
  uint32_t a, b, c, d;
  __asm__ __volatile__(
    "cpuid"
    : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
    : "a"(0)
    : "memory");
}
/* clang-format on */

/**
 * @brief Prepares the execution environment for high-precision timing.
 *
 * Drains the pipeline and serializes execution before capturing the TSC.
 *
 * @return Starting 64-bit cycle count.
 */
_CACHE_INLINE uint64_t probe_start(void) {
  cpuid();
  return rdtsc();
}

/**
 * @brief Captures the end cycle count and serializes the pipeline.
 *
 * Ensures all monitored work is completed before capturing the TSC, then
 * drains the pipeline again to prevent subsequent instructions from leaking
 * into the timed interval.
 *
 * @return Ending 64-bit cycle count.
 */
_CACHE_INLINE uint64_t probe_end(void) {
  uint64_t t = rdtscp();
  cpuid();
  return t;
}

/**
 * @brief Lightweight version of probe_start using lfence.
 *
 * Faster than probe_start but only serializes memory instructions.
 *
 * @return Starting 64-bit cycle count.
 */
_CACHE_INLINE uint64_t probe_start_lfence(void) {
  lfence();
  return rdtsc();
}

/**
 * @brief Lightweight version of probe_end using lfence.
 *
 * Faster than probe_end but only serializes memory instructions.
 *
 * @return Ending 64-bit cycle count.
 */
_CACHE_INLINE uint64_t probe_end_lfence(void) {
  uint64_t t = rdtscp();
  lfence();
  return t;
}

/**
 * @brief Forces a memory load from the specified address.
 *
 * This is an idempotent operation used to pull data into the cache hierarchy.
 *
 * @param p Pointer to the memory location to access.
 */
_CACHE_INLINE void maccess(const void *p) {
  __asm__ __volatile__("movq (%0), %%rax" : : "r"(p) : "rax");
}

/**
 * @brief Flushes the cache line containing the specified address.
 *
 * Invalidates the data in all levels of the cache hierarchy.
 *
 * @param p Pointer to the memory location to flush.
 */
_CACHE_INLINE void clflush(const void *p) {
  __asm__ __volatile__("clflush (%0)" ::"r"(p) : "memory");
}

#undef _CACHE_INLINE

#endif /* AXIUM_CACHE_H */
