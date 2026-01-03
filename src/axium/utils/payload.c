#include <axium/utils/payload.h>
#include <string.h>

void patch(uint8_t *restrict buf, size_t buf_size, const void *restrict marker,
           size_t marker_size, const void *restrict replacement,
           size_t replacement_size) {
  if (__builtin_expect(marker_size == 0 || buf_size < marker_size, 0)) {
    return;
  }

  const size_t copy_len =
      (replacement_size < marker_size) ? replacement_size : marker_size;
  const size_t zero_len = marker_size - copy_len;

  uint8_t *curr = buf;
  const uint8_t *const end = buf + buf_size;
  const uint8_t *const limit = end - marker_size;

  // Optimized path for single-byte markers
  if (marker_size == 1) {
    const uint8_t m = *(const uint8_t *)marker;
    const uint8_t r = *(const uint8_t *)replacement;
    while (curr <= limit) {
      curr = memchr(curr, m, (size_t)(end - curr));
      if (!curr)
        break;
      *curr = r;
      curr++;
    }
    return;
  }

  // General path for multi-byte markers
  while (curr <= limit) {
    curr = memmem(curr, (size_t)(end - curr), marker, marker_size);
    if (!curr)
      break;

    memcpy(curr, replacement, copy_len);
    if (zero_len > 0) {
      memset(curr + copy_len, 0, zero_len);
    }
    curr += marker_size;
  }
}
