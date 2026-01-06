#ifndef AXIUM_PAYLOAD_H
#define AXIUM_PAYLOAD_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Dynamic buffer structure for payload construction.
 */
typedef struct {
  uint8_t *data;   /**< Pointer to the allocated buffer */
  size_t size;     /**< Current size of the payload in bytes */
  size_t capacity; /**< Total allocated capacity in bytes */
} payload_t;

/** @brief Stringify a macro argument */
#define STR(x) #x
/** @brief Expand and stringify a macro argument */
#define XSTR(x) STR(x)

/**
 * @brief Initialize a payload structure.
 * @param p Pointer to the payload_t structure to initialize.
 */
void payload_init(payload_t *p) __attribute__((nonnull(1)));

/**
 * @brief Destory a payload structure, freeing its data.
 * @param p Pointer to the payload_t structure to deinitialize.
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

/** @brief Append an 8-bit integer to the payload. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u8(payload_t *p, uint8_t val) {
  payload_push(p, &val, sizeof(val));
}

/** @brief Append a 16-bit integer to the payload. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u16(payload_t *p, uint16_t val) {
  payload_push(p, &val, sizeof(val));
}

/** @brief Append a 32-bit integer to the payload. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u32(payload_t *p, uint32_t val) {
  payload_push(p, &val, sizeof(val));
}

/** @brief Append a 64-bit integer to the payload. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_push_u64(payload_t *p, uint64_t val) {
  payload_push(p, &val, sizeof(val));
}

/**
 * @brief Variadic macro to push multiple 64-bit integers to the payload.
 * Typically used for ROP chains on 64-bit architectures (e.g., x86_64).
 */
#define PAYLOAD_PUSH_U64S(p, ...)                                              \
  payload_push(p, (const uint64_t[]){__VA_ARGS__},                             \
               sizeof((const uint64_t[]){__VA_ARGS__}))

/**
 * @brief Variadic macro to push multiple 32-bit integers to the payload.
 * Typically used for ROP chains on 32-bit architectures (e.g., x86).
 */
#define PAYLOAD_PUSH_U32S(p, ...)                                              \
  payload_push(p, (const uint32_t[]){__VA_ARGS__},                             \
               sizeof((const uint32_t[]){__VA_ARGS__}))

/**
 * @brief Search and replace a data pattern within a raw buffer.
 * @param buf Buffer to patch.
 * @param buf_size Size of the buffer.
 * @param marker Pattern to search for.
 * @param marker_size Size of the marker.
 * @param replacement Data to replace with.
 * @param replacement_size Size of replacement data.
 */
void patch(uint8_t *restrict buf, size_t buf_size, const void *restrict marker,
           size_t marker_size, const void *restrict replacement,
           size_t replacement_size) __attribute__((nonnull(1, 3, 5)));

/** @brief Patch an 8-bit marker with a new value. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u8(payload_t *p, uint8_t marker, uint8_t replacement) {
  patch(p->data, p->size, &marker, sizeof(marker), &replacement,
        sizeof(replacement));
}

/** @brief Patch a 16-bit marker with a new value. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u16(payload_t *p, uint16_t marker, uint16_t replacement) {
  patch(p->data, p->size, &marker, sizeof(marker), &replacement,
        sizeof(replacement));
}

/** @brief Patch a 32-bit marker with a new value. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u32(payload_t *p, uint32_t marker, uint32_t replacement) {
  patch(p->data, p->size, &marker, sizeof(marker), &replacement,
        sizeof(replacement));
}

/** @brief Patch a 64-bit marker with a new value. */
static inline __attribute__((always_inline, nonnull(1))) void
payload_patch_u64(payload_t *p, uint64_t marker, uint64_t replacement) {
  patch(p->data, p->size, &marker, sizeof(marker), &replacement,
        sizeof(replacement));
}

/**
 * @brief Patch 8-bit relative offsets for each occurrence of a marker.
 * @param p Pointer to the payload.
 * @param marker 8-bit marker to search for.
 * @param target_offset Absolute target offset relative to payload start.
 */
void payload_patch_rel8(payload_t *p, uint8_t marker, size_t target_offset)
    __attribute__((nonnull(1)));

/**
 * @brief Patch 16-bit relative offsets for each occurrence of a marker.
 * @param p Pointer to the payload.
 * @param marker 16-bit marker to search for.
 * @param target_offset Absolute target offset relative to payload start.
 */
void payload_patch_rel16(payload_t *p, uint16_t marker, size_t target_offset)
    __attribute__((nonnull(1)));

/**
 * @brief Patch 32-bit relative offsets for each occurrence of a marker.
 * @param p Pointer to the payload.
 * @param marker 32-bit marker to search for.
 * @param target_offset Absolute target offset relative to payload start.
 */
void payload_patch_rel32(payload_t *p, uint32_t marker, size_t target_offset)
    __attribute__((nonnull(1)));

/**
 * @brief Patch 64-bit relative offsets for each occurrence of a marker.
 * @param p Pointer to the payload.
 * @param marker 64-bit marker to search for.
 * @param target_offset Absolute target offset relative to payload start.
 */
void payload_patch_rel64(payload_t *p, uint64_t marker, size_t target_offset)
    __attribute__((nonnull(1)));

#endif // AXIUM_PAYLOAD_H
