#include <axium/log.h>
#include <axium/sidechannel/cpu.h>
#include <errno.h>
#include <sys/syscall.h>

void set_pid_cpu_affinity(pid_t pid, int cpu_core) {
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(cpu_core, &cpu_set);

  long ret;
#if defined(__x86_64__)
  /* clang-format off */
  __asm__ __volatile__(
    "syscall"
    : "=a"(ret)
    : "a"(SYS_sched_setaffinity), "D"(pid),
      "S"(sizeof(cpu_set)), "d"(&cpu_set)
    : "rcx", "r11", "memory"
  );
  /* clang-format on */
  if (ret < 0 && ret > -4096) {
    errno = -ret;
    ret = -1;
  }
#else
  ret = syscall(SYS_sched_setaffinity, pid, sizeof(cpu_set), &cpu_set);
#endif
  if (__builtin_expect(ret == -1, 0)) {
    log_exception("Failed to set CPU affinity for PID %d to core %d", pid,
                  cpu_core);
  }

  log_debug("Successfully set CPU affinity for PID %d to core %d", pid,
            cpu_core);
}
