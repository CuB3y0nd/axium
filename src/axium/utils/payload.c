#include <axium/utils/payload.h>
#include <stdbool.h>
#include <stdlib.h>

void payload_init(payload_t *p) {
  p->data = NULL;
  p->size = 0;
  p->capacity = 0;
}

void payload_fini(payload_t *p) {
  free(p->data);
  p->data = NULL;
  p->size = 0;
  p->capacity = 0;
}

/** Internal helper to ensure buffer capacity. */
static inline bool ensure_capacity(payload_t *p, size_t needed) {
  if (needed <= p->capacity)
    return true;

  size_t new_cap = p->capacity ? p->capacity * 2 : 256;
  while (new_cap < needed) {
    new_cap *= 2;
  }
  uint8_t *new_data = realloc(p->data, new_cap);
  if (!new_data)
    return false;

  p->data = new_data;
  p->capacity = new_cap;
  return true;
}

void payload_push(payload_t *p, const void *data, size_t size) {
  if (size == 0 || !ensure_capacity(p, p->size + size))
    return;

  memcpy(p->data + p->size, data, size);
  p->size += size;
}

void payload_push_str(payload_t *p, const char *s) {
  payload_push(p, s, strlen(s));
}

void payload_fill_to(payload_t *p, size_t offset, const void *filler,
                     size_t filler_size) {
  if (p->size >= offset || !ensure_capacity(p, offset))
    return;

  size_t diff = offset - p->size;
  uint8_t *dest = p->data + p->size;

  if (!filler || filler_size == 0) {
    memset(dest, 0, diff);
  } else if (filler_size == 1) {
    memset(dest, *(const uint8_t *)filler, diff);
  } else {
    /* Pattern fill */
    size_t copied = 0;
    while (copied < diff) {
      size_t chunk =
          (diff - copied < filler_size) ? (diff - copied) : filler_size;
      memcpy(dest + copied, filler, chunk);
      copied += chunk;
    }
  }
  p->size = offset;
}

/** Internal helper for relative patching. */
static void __attribute__((hot, nonnull(1, 2)))
patch_rel_generic(payload_t *p, const void *marker, size_t marker_size,
                  size_t target_offset) {
  if (!p->data || p->size < marker_size)
    return;

  uint8_t *curr = p->data;
  const uint8_t *const end = p->data + p->size;
  const uint8_t *const limit = end - marker_size;

  while (curr <= limit) {
    curr = memmem(curr, (size_t)(end - curr), marker, marker_size);
    if (!curr)
      break;

    const size_t marker_off = (size_t)(curr - p->data);

    switch (marker_size) {
    case 1: {
      uint8_t disp = (uint8_t)(target_offset - (marker_off + 1));
      memcpy(curr, &disp, 1);
      break;
    }
    case 2: {
      uint16_t disp = (uint16_t)(target_offset - (marker_off + 2));
      memcpy(curr, &disp, 2);
      break;
    }
    case 4: {
      uint32_t disp = (uint32_t)(target_offset - (marker_off + 4));
      memcpy(curr, &disp, 4);
      break;
    }
    case 8: {
      uint64_t disp = (uint64_t)(target_offset - (marker_off + 8));
      memcpy(curr, &disp, 8);
      break;
    }
    }

    curr += marker_size;
  }
}

void payload_patch_rel8(payload_t *p, uint8_t marker, size_t target_offset) {
  patch_rel_generic(p, &marker, 1, target_offset);
}

void payload_patch_rel16(payload_t *p, uint16_t marker, size_t target_offset) {
  patch_rel_generic(p, &marker, 2, target_offset);
}

void payload_patch_rel32(payload_t *p, uint32_t marker, size_t target_offset) {
  patch_rel_generic(p, &marker, 4, target_offset);
}

void payload_patch_rel64(payload_t *p, uint64_t marker, size_t target_offset) {
  patch_rel_generic(p, &marker, 8, target_offset);
}

void __attribute__((hot)) patch(uint8_t *restrict buf, size_t buf_size,
                                const void *restrict marker, size_t marker_size,
                                const void *restrict replacement,
                                size_t replacement_size) {
  if (marker_size == 0 || buf_size < marker_size)
    return;
  const size_t copy_len =
      (replacement_size < marker_size) ? replacement_size : marker_size;
  const size_t zero_len = marker_size - copy_len;
  uint8_t *curr = buf;
  const uint8_t *const end = buf + buf_size;
  const uint8_t *const limit = end - marker_size;

  while (curr <= limit) {
    curr = memmem(curr, (size_t)(end - curr), marker, marker_size);
    if (!curr)
      break;
    if (copy_len > 0)
      memcpy(curr, replacement, copy_len);
    if (zero_len > 0)
      memset(curr + copy_len, 0, zero_len);
    curr += marker_size;
  }
}
