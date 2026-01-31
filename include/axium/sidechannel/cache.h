/**
 * @file cache.h
 *
 * Provides high-performance primitives and analysis tools for cache
 * side-channel exploitation.
 */

#ifndef AXIUM_CACHE_H
#define AXIUM_CACHE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Shuffles an index using a Linear Congruential Mapping to defeat
 * hardware prefetchers.
 *
 * Formula: `(i * 167 + 13) & mask`.
 *
 * @param i The original sequential index.
 * @param mask The bitwise mask.
 *
 * > [!IMPORTANT]
 * > MUST be `(ARRAY_SIZE - 1)`, where `ARRAY_SIZE` is a power of 2.
 */
#define MIXED_IDX(i, mask) (((i) * 167 + 13) & (mask))

/**
 * @brief Internal attribute for timing-critical inlining.
 */
#define _CACHE_INLINE static inline __attribute__((always_inline))

/**
 * @brief Reads the Time Stamp Counter (TSC).
 *
 * Raw, non-serializing read of the TSC.
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
 * Partially serializing: waits for preceding instructions to finish.
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
 * Drains the pipeline to prevent speculative execution.
 */
_CACHE_INLINE void cpuid(void) {
  uint32_t a, b, c, d;
  __asm__ __volatile__(
    "cpuid"
    : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
    : "a"(0)
    : "memory"
  );
}
/* clang-format on */

/**
 * @brief Serializing start probe.
 * @return Starting cycle count.
 */
_CACHE_INLINE uint64_t probe_start(void) {
  cpuid();
  return rdtsc();
}

/**
 * @brief Serializing end probe.
 * @return Ending cycle count.
 */
_CACHE_INLINE uint64_t probe_end(void) {
  uint64_t t = rdtscp();
  cpuid();
  return t;
}

/**
 * @brief Lightweight version of probe_start using lfence.
 *
 * Probably faster than probe_start but only serializes memory instructions.
 *
 * @return Starting cycle count.
 */
_CACHE_INLINE uint64_t probe_start_lfence(void) {
  lfence();
  return rdtsc();
}

/**
 * @brief Lightweight version of probe_end using lfence.
 *
 * Probably faster than probe_end but only serializes memory instructions.
 *
 * @return Ending cycle count.
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

/**
 * @brief Detects the CPU's CLFLUSH line size at runtime.
 * @return Cache line size in bytes (usually 64).
 */
_CACHE_INLINE size_t cache_line_size(void) {
  uint32_t a, b, c, d;
  __asm__ __volatile__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(1));
  return ((b >> 8) & 0xFF) * 8;
}

/**
 * @brief Flushes a range of memory from the cache using dynamic line size.
 *
 * @param p Pointer to the memory location to flush.
 * @param size Total size of the range being flushed.
 */
_CACHE_INLINE void cache_flush_range(const void *p, size_t size) {
  const char *ptr = (const char *)p;
  size_t step = cache_line_size();
  if (step == 0)
    step = 64; /* Fallback to typical value */
  for (size_t i = 0; i < size; i += step) {
    clflush(ptr + i);
  }
}

/**
 * @brief Automatically calibrates the cache hit/miss threshold.
 *
 * @param target Optional pointer to the memory region being probed.
 * @return Recommended cycle threshold.
 */
uint64_t cache_calibrate_threshold(const void *target);

/**
 * @brief Results and analysis structure.
 */
typedef struct {
  uint64_t threshold;           /**< Original calibrated threshold. */
  uint64_t effective_threshold; /**< Refined threshold after analysis. */
  uint64_t *timings;            /**< Raw timing results. */
  size_t count;                 /**< Number of elements. */
  int winner_idx;               /**< Index of the global minimum. */
  uint64_t winner_val;          /**< Timing of the winner. */
  uint64_t gap;                 /**< Timing gap between winner and runner-up. */
  size_t hits_count;            /**< Refined count of valid hits. */
} cache_report_t;

/**
 * @brief Analyzes timing data using Gap-based noise reduction.
 *
 * @param report Pointer to the report structure to populate.
 * @param timings Array of raw timing measurements.
 * @param count Number of elements in the timings array.
 * @param threshold Initial cycle threshold for hit detection.
 */
void cache_analyze(cache_report_t *report, uint64_t *timings, size_t count,
                   uint64_t threshold);

#undef _CACHE_INLINE

#endif /* AXIUM_CACHE_H */
