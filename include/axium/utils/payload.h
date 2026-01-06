#ifndef AXIUM_PAYLOAD_H
#define AXIUM_PAYLOAD_H

#include <axium/utils/packing.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Dynamic buffer structure for payload construction.
 */
typedef struct {
  uint8_t *data;   /**< Pointer to the allocated buffer. */
  size_t size;     /**< Current size of the payload in bytes. */
  size_t capacity; /**< Total allocated capacity in bytes. */
} payload_t;

/**
 * @brief Initialize a payload structure.
 * @param p Pointer to the payload_t structure to initialize.
 */
void payload_init(payload_t *p) __attribute__((nonnull(1)));

/**
 * @brief Destroy a payload structure, freeing its data.
 * @param p Pointer to the payload_t structure to finalize.
 */
void payload_fini(payload_t *p) __attribute__((nonnull(1)));

/**
 * @brief Append raw data to the payload.
 * @param p Pointer to the payload structure.
 * @param data Data to push.
 * @param size Size of the data in bytes.
 */
void payload_push(payload_t *p, const void *data, size_t size)
    __attribute__((nonnull(1, 2)));

/**
 * @brief Append a null-terminated string to the payload (excluding null byte).
 * @param p Pointer to the payload.
 * @param s String to push.
 */
void payload_push_str(payload_t *p, const char *s)
    __attribute__((nonnull(1, 2)));

/**
 * @brief Fill the payload up to a specific offset.
 *
 * If the target offset is greater than the current size, the gap is filled
 * using the provided pattern.
 *
 * @param p Pointer to the payload.
 * @param offset Target offset relative to payload start.
 * @param filler Pattern to use for filling. If NULL or filler_size is 0, \x00
 * is used.
 * @param filler_size Size of the filler pattern in bytes.
 */
void payload_fill_to(payload_t *p, size_t offset, const void *filler,
                     size_t filler_size) __attribute__((nonnull(1)));

/**
 * @brief Search and replace a data pattern within the payload buffer.
 *
 * This function supports an "Erase Mode": if 'replacement' is NULL and
 * 'replacement_size' is 0, all occurrences of 'marker' will be zero-filled.
 *
 * @param buf Buffer to patch.
 * @param buf_size Size of the buffer.
 * @param marker Pattern to search for.
 * @param marker_size Size of the marker.
 * @param replacement Data to replace with. Can be NULL if replacement_size is
 * 0.
 * @param replacement_size Size of replacement data. If smaller than marker,
 * the rest is zeroed.
 */
void patch(uint8_t *restrict buf, size_t buf_size, const void *restrict marker,
           size_t marker_size, const void *restrict replacement,
           size_t replacement_size) __attribute__((nonnull(1, 3)));

/**
 * @brief Push an 8-bit integer.
 * @param p Payload pointer.
 * @param val Value to push.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u8(payload_t *p, uint8_t val) {
  payload_push(p, &val, 1);
}

/**
 * @brief Push a 16-bit integer.
 * @param p Payload pointer.
 * @param val Value to push.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u16(payload_t *p, uint16_t val) {
  payload_push(p, &val, 2);
}

/**
 * @brief Push a 32-bit integer.
 * @param p Payload pointer.
 * @param val Value to push.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u32(payload_t *p, uint32_t val) {
  payload_push(p, &val, 4);
}

/**
 * @brief Push a 64-bit integer.
 * @param p Payload pointer.
 * @param val Value to push.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u64(payload_t *p, uint64_t val) {
  payload_push(p, &val, 8);
}

/**
 * @brief Place a string at a specific offset.
 * @param p Payload pointer.
 * @param offset Offset from start.
 * @param s String to place.
 */
static inline __attribute__((always_inline, nonnull(1, 3))) void
payload_at_str(payload_t *p, size_t offset, const char *s) {
  payload_fill_to(p, offset, NULL, 0);
  payload_push_str(p, s);
}

/**
 * @brief Place an 8-bit integer at a specific offset.
 * @param p Payload pointer.
 * @param offset Offset from start.
 * @param val Value to place.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_at_u8(payload_t *p, size_t offset, uint8_t val) {
  payload_fill_to(p, offset, NULL, 0);
  payload_push_u8(p, val);
}

/**
 * @brief Place a 16-bit integer at a specific offset.
 * @param p Payload pointer.
 * @param offset Offset from start.
 * @param val Value to place.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_at_u16(payload_t *p, size_t offset, uint16_t val) {
  payload_fill_to(p, offset, NULL, 0);
  payload_push_u16(p, val);
}

/**
 * @brief Place a 32-bit integer at a specific offset.
 * @param p Payload pointer.
 * @param offset Offset from start.
 * @param val Value to place.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_at_u32(payload_t *p, size_t offset, uint32_t val) {
  payload_fill_to(p, offset, NULL, 0);
  payload_push_u32(p, val);
}

/**
 * @brief Place a 64-bit integer at a specific offset.
 * @param p Payload pointer.
 * @param offset Offset from start.
 * @param val Value to place.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_at_u64(payload_t *p, size_t offset, uint64_t val) {
  payload_fill_to(p, offset, NULL, 0);
  payload_push_u64(p, val);
}

/**
 * @brief Patch an 8-bit marker.
 * @param p Payload pointer.
 * @param marker Marker to find.
 * @param replacement Value to replace with.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u8(payload_t *p, uint8_t marker, uint8_t replacement) {
  patch(p->data, p->size, &marker, 1, &replacement, 1);
}

/**
 * @brief Patch a 16-bit marker.
 * @param p Payload pointer.
 * @param marker Marker to find.
 * @param replacement Value to replace with.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u16(payload_t *p, uint16_t marker, uint16_t replacement) {
  uint16_t m = marker, r = replacement;
  patch(p->data, p->size, &m, 2, &r, 2);
}

/**
 * @brief Patch a 32-bit marker.
 * @param p Payload pointer.
 * @param marker Marker to find.
 * @param replacement Value to replace with.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u32(payload_t *p, uint32_t marker, uint32_t replacement) {
  uint32_t m = marker, r = replacement;
  patch(p->data, p->size, &m, 4, &r, 4);
}

/**
 * @brief Patch a 64-bit marker.
 * @param p Payload pointer.
 * @param marker Marker to find.
 * @param replacement Value to replace with.
 */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u64(payload_t *p, uint64_t marker, uint64_t replacement) {
  uint64_t m = marker, r = replacement;
  patch(p->data, p->size, &m, 8, &r, 8);
}

/**
 * @brief Push a packed array of values.
 *
 * This macro leverages C compound literals. Supports designated initializers.
 * Example: PAYLOAD_PACK(p, uint64_t, [0]=gadget, [5]=addr)
 *
 * @param p Pointer to the payload.
 * @param type The type of each element (e.g., uint64_t).
 * @param ... Elements or designated initializers.
 */
#define PAYLOAD_PACK(p, type, ...)                                             \
  payload_push(p, (const type[]){__VA_ARGS__},                                 \
               sizeof((const type[]){__VA_ARGS__}))

/** @brief Patch 8-bit relative displacement for marker. */
void payload_patch_rel8(payload_t *p, uint8_t marker, size_t target_offset);
/** @brief Patch 16-bit relative displacement for marker. */
void payload_patch_rel16(payload_t *p, uint16_t marker, size_t target_offset);
/** @brief Patch 32-bit relative displacement for marker. */
void payload_patch_rel32(payload_t *p, uint32_t marker, size_t target_offset);
/** @brief Patch 64-bit relative displacement for marker. */
void payload_patch_rel64(payload_t *p, uint64_t marker, size_t target_offset);

#endif // AXIUM_PAYLOAD_H
