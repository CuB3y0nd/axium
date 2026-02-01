/**
 * @file cpu.h
 *
 * Provides CPU affinity utilities for side-channel attacks.
 */

#ifndef AXIUM_SIDECHANNEL_CPU_H
#define AXIUM_SIDECHANNEL_CPU_H

#include <sched.h>
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

#endif // AXIUM_SIDECHANNEL_CPU_H
