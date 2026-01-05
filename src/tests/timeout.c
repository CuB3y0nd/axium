#include <axium/log.h>
#include <axium/timeout.h>
#include <axium/tubes/process.h>
#include <axium/tubes/tube.h>
#include <test_common.h>

void test_timeout_basic_logic(void) {
  printf("--- Testing basic timeout logic and constants ---\n");

  ASSERT_TRUE(TIMEOUT_FOREVER < 0, "TIMEOUT_FOREVER should be negative");
  ASSERT_TRUE(TIMEOUT_DEFAULT < 0, "TIMEOUT_DEFAULT should be negative");
  ASSERT_TRUE(TIMEOUT_DEFAULT != TIMEOUT_FOREVER,
              "Sentinels should be distinct");

  double t1 = timeout_now();
  usleep(10000); // 10ms
  double t2 = timeout_now();

  ASSERT_TRUE(t2 > t1, "timeout_now should be monotonic");
  ASSERT_TRUE(t2 - t1 >= 0.01, "time difference should be at least 10ms");
}

void test_tube_specific_timeout(void) {
  printf("--- Testing specific timeout on tube ---\n");

  // Spawn a process that sleeps longer than our timeout
  char *argv[] = {"/bin/sh", "-c", "sleep 2; echo 'DONE'", NULL};
  tube *t = process(argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn process");

  double start = timeout_now();
  char buf[10];
  ssize_t n = recv(t, buf, sizeof(buf), 0.1); // 100ms timeout
  double end = timeout_now();

  ASSERT_TRUE(n == 0, "recv should timeout and return 0");
  ASSERT_TRUE(end - start < 0.5, "should return near 100ms, not 2s");

  t_close(t);
}

void test_tube_default_timeout(void) {
  printf("--- Testing default timeout (TIMEOUT_DEFAULT) ---\n");

  char *argv[] = {"/bin/sh", "-c", "sleep 2; echo 'DONE'", NULL};
  tube *t = process(argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn process");

  // Set default timeout for this tube
  t->timeout = 0.2;

  double start = timeout_now();
  void *res = recvuntil(t, "DONE", TIMEOUT_DEFAULT, NULL);
  double end = timeout_now();

  ASSERT_TRUE(res == NULL, "recvuntil should timeout based on default");
  ASSERT_TRUE(end - start < 0.5, "should respect t->timeout");

  t_close(t);
}

void test_recvuntil_countdown(void) {
  printf("--- Testing recvuntil countdown logic ---\n");

  // Process outputs PART1, waits 0.5s, then PART2
  char *argv[] = {"/bin/sh", "-c",
                  "echo -n 'PART1'; sleep 0.5; echo -n 'PART2'", NULL};
  tube *t = process(argv, NULL);

  // We wait for PART2 with a 0.2s timeout.
  // It should successfully read PART1, then timeout waiting for PART2.
  size_t sz = 0;
  void *res = recvuntil(t, "PART2", 0.2, &sz);

  ASSERT_TRUE(res != NULL, "Should return what it managed to read");
  ASSERT_STREQ(res, "PART1", "Should have captured PART1 before timeout");
  ASSERT_TRUE(sz == 5, "Size should match PART1");
  t_free(res);

  t_close(t);
}

void test_recvall_timeout(void) {
  printf("--- Testing recvall timeout ---\n");

  char *argv[] = {"/bin/sh", "-c", "echo -n 'START'; sleep 5", NULL};
  tube *t = process(argv, NULL);

  size_t sz = 0;
  void *res = recvall(t, 0.5, &sz);

  ASSERT_TRUE(res != NULL, "recvall should return partial data on timeout");
  ASSERT_STREQ(res, "START", "captured data matches");
  // recvall automatically closes the tube on completion or timeout
}

int main(void) {
  set_log_level(ERROR);
  test_timeout_basic_logic();
  test_tube_specific_timeout();
  test_tube_default_timeout();
  test_recvuntil_countdown();
  test_recvall_timeout();
  return 0;
}
