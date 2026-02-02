/**
 * @file oracle.c
 *
 * @brief Implementation of generic oracle utilities.
 */

#include <axium/oracle.h>

ssize_t oracle_scan(oracle_t *o, char *buf, size_t len, int terminator) {
  if (!o || !buf) {
    return -1;
  }

  size_t i;
  for (i = 0; i < len; i++) {
    int res = oracle_query(o, i);
    if (res == -1) {
      break;
    }

    buf[i] = (char)res;

    if (terminator >= 0 && res == terminator) {
      i++; /* Include the terminator in the count */
      break;
    }
  }

  return (ssize_t)i;
}
