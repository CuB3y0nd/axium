#include <axium/axium.h>
#include <test_common.h>

void test_set_cpu_affinity() {
  int core = 0;
  set_cpu_affinity(core);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
    log_exception("sched_getaffinity");
  }

  ASSERT_TRUE(CPU_ISSET(core, &mask), "CPU affinity should be set to core 0");
}

void test_set_cpu_affinity_failure() {
  int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  ASSERT_EXIT_FAIL(
      { set_cpu_affinity(nprocs); },
      "Setting affinity to a non-existent core should exit the program");
}

void test_set_cpu_affinity_invalid_pid() {
  ASSERT_EXIT_FAIL(
      { set_pid_cpu_affinity(999999, 0); },
      "Setting affinity for an invalid PID should exit the program");
}

int main() {
  set_log_level(INFO);
  test_set_cpu_affinity();
  test_set_cpu_affinity_failure();
  test_set_cpu_affinity_invalid_pid();
  return 0;
}
