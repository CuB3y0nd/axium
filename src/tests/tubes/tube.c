#include <axium/log.h>
#include <axium/tubes/process.h>
#include <axium/tubes/tube.h>
#include <test_common.h>

void test_tube_recvuntil_and_line(void) {
  printf("--- Testing recvuntil and recvline ---\n");
  char *argv[] = {"/bin/sh", "-c",
                  "echo 'PART1:PART2:END'; read input; echo \"RECVD:$input\"",
                  NULL};

  tube *t = process(argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn shell process");

  size_t sz;
  // Test recvuntil
  char *p1 = recvuntil(t, ":", TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(p1 != NULL, "recvuntil ':' returns data");
  ASSERT_TRUE(sz == 6, "recvuntil ':' size is 6");
  ASSERT_STREQ(p1, "PART1:", "recvuntil ':' matched PART1:");
  t_free(p1);

  char *p2 = recvuntil(t, ":", TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(p2 != NULL, "recvuntil ':' second call returns data");
  ASSERT_TRUE(sz == 6, "recvuntil ':' size is 6");
  ASSERT_STREQ(p2, "PART2:", "recvuntil ':' matched PART2:");
  t_free(p2);

  // Test recvline
  char *p3 = recvline(t, TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(p3 != NULL, "recvline returns data");
  ASSERT_STREQ(p3, "END\n", "recvline matched 'END\n'");
  t_free(p3);

  // Test sendline
  sendline(t, "AXIUM_TUBE_TEST", 15);

  // Test response
  char *p4 = recvline(t, TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(p4 != NULL, "recvline for response returns data");
  ASSERT_STREQ(p4, "RECVD:AXIUM_TUBE_TEST\n",
               "response matches expected input");
  t_free(p4);

  t_close(t);
}

void test_tube_recvlines(void) {
  printf("--- Testing recvlines ---\n");
  char *argv[] = {"/bin/sh", "-c", "echo 'Line 1\nLine 2\nLine 3'", NULL};
  tube *t = process(argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn process for recvlines");

  size_t count;
  void **lines = recvlines(t, 2, TIMEOUT_DEFAULT, &count);
  ASSERT_TRUE(lines != NULL, "recvlines returns pointer array");
  ASSERT_TRUE(count == 2, "recvlines count is 2");
  ASSERT_STREQ((char *)lines[0], "Line 1\n", "Line 1 matches");
  ASSERT_STREQ((char *)lines[1], "Line 2\n", "Line 2 matches");
  ASSERT_TRUE(lines[2] == NULL, "Array is NULL-terminated");

  t_freelines(lines);
  t_close(t);
}

void test_tube_recvall(void) {
  printf("--- Testing recvall ---\n");
  char *argv[] = {"/bin/sh", "-c", "echo -n 'ALL_DATA_EOF'", NULL};
  tube *t = process(argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn process for recvall");

  size_t sz;
  char *data = recvall(t, TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(data != NULL, "recvall returns data");
  ASSERT_TRUE(sz == 12, "recvall size is 12");
  ASSERT_STREQ(data, "ALL_DATA_EOF", "recvall content matches");

  t_free(data);
  // recvall closes the tube automatically
}

void test_tube_send_after_then(void) {
  printf("--- Testing sendafter/sendthen family ---\n");
  char *argv[] = {
      "/bin/sh", "-c",
      "echo -n 'PROMPT: '; read input; echo \"GOT:$input\"; echo -n 'NEXT: '",
      NULL};
  tube *t = process(argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn process for sendafter/then");

  size_t sz;
  // Use sendlineafter instead of sendafter to satisfy shell's 'read'
  char *res1 = sendlineafter(t, "PROMPT: ", "INPUT1", 6, TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(res1 != NULL, "sendlineafter returns prompt data");
  ASSERT_STREQ(res1, "PROMPT: ", "res1 matches prompt");
  t_free(res1);

  // Test sendthen
  char *res2 = sendthen(t, "NEXT: ", "INPUT2", 6, TIMEOUT_DEFAULT, &sz);
  ASSERT_TRUE(res2 != NULL, "sendthen returns response data");
  ASSERT_TRUE(strstr(res2, "GOT:INPUT1") != NULL, "res2 contains GOT:INPUT1");
  ASSERT_TRUE(strstr(res2, "NEXT: ") != NULL, "res2 contains NEXT: ");
  t_free(res2);

  t_close(t);
}

void test_tube_raw_io(void) {
  printf("--- Testing send and recv with cat ---\n");
  char *cat_argv[] = {"/bin/cat", NULL};
  tube *t = process(cat_argv, NULL);
  ASSERT_TRUE(t != NULL, "Spawn cat process");

  const char *raw_data = "\x01\x02\x03\x04\x00\xff";
  send(t, raw_data, 6);

  char recv_buf[6] = {0};
  ssize_t n = recv(t, recv_buf, 6, TIMEOUT_DEFAULT);
  ASSERT_TRUE(n == 6, "recv received 6 bytes");
  ASSERT_TRUE(memcmp(recv_buf, raw_data, 6) == 0,
              "recv data matches raw input");

  t_close(t);
}

void test_tube_interactive(void) {
  printf("--- interactive test (skipped in automated tests usually) "
         "---\n");
  /* We don't actually run it here because it's blocking and requires user
   * input, but we compile it to ensure API consistency. */
  if (0) {
    char *argv[] = {"/bin/sh", NULL};
    tube *t = process(argv, NULL);
    interactive(t, NULL);
  }
}

int main(void) {
  set_log_level(ERROR); // Quiet down for tests
  test_tube_recvuntil_and_line();
  test_tube_recvlines();
  test_tube_recvall();
  test_tube_send_after_then();
  test_tube_raw_io();
  test_tube_interactive();
  return 0;
}
