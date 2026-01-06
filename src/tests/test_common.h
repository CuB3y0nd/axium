#ifndef AXIUM_TEST_COMMON_H
#define AXIUM_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_TRUE(condition, message)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: %s at %s:%d - %s\n", #condition,      \
              __FILE__, __LINE__, message);                                    \
      exit(EXIT_FAILURE);                                                      \
    } else {                                                                   \
      printf("%s ... ok\n", message);                                          \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(actual, expected, message)                                   \
  do {                                                                         \
    if ((actual) != (expected)) {                                              \
      fprintf(stderr,                                                          \
              "Assertion failed (EQ):\n  Actual:   %zu (0x%zx)\n  Expected: "  \
              "%zu (0x%zx)\n  at %s:%d - %s\n",                                \
              (size_t)(actual), (size_t)(actual), (size_t)(expected),          \
              (size_t)(expected), __FILE__, __LINE__, message);                \
      exit(EXIT_FAILURE);                                                      \
    } else {                                                                   \
      printf("%s ... ok\n", message);                                          \
    }                                                                          \
  } while (0)

#define ASSERT_STREQ(actual, expected, message)                                \
  do {                                                                         \
    if (strcmp(actual, expected) != 0) {                                       \
      fprintf(stderr,                                                          \
              "Assertion failed (STREQ):\n  Actual:   \"%s\"\n  Expected: "    \
              "\"%s\"\n  at %s:%d - %s\n",                                     \
              actual, expected, __FILE__, __LINE__, message);                  \
      exit(EXIT_FAILURE);                                                      \
    } else {                                                                   \
      printf("%s ... ok\n", message);                                          \
    }                                                                          \
  } while (0)

#endif // AXIUM_TEST_COMMON_H