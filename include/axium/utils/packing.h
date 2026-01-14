/**
 * @file packing.h
 *
 * Provides endian-aware integer packing and unpacking utilities.
 *
 * This file contains functions for converting between `host-order` integers and
 * `little-endian`/`big-endian` representations.
 */

#ifndef AXIUM_PACKING_H
#define AXIUM_PACKING_H

#include <stdint.h>
#include <string.h>

/* --- Endianness detection --- */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define AXIUM_IS_BIG_ENDIAN 1
#else
#define AXIUM_IS_BIG_ENDIAN 0
#endif

/**
 * Byte-swaps a 16-bit integer.
 *
 * @param v 16-bit value to swap.
 * @return Byte-swapped 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t
__swap16(uint16_t v) {
  return __builtin_bswap16(v);
}

/**
 * Byte-swaps a 32-bit integer.
 *
 * @param v 32-bit value to swap.
 * @return Byte-swapped 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t
__swap32(uint32_t v) {
  return __builtin_bswap32(v);
}

/**
 * Byte-swaps a 64-bit integer.
 *
 * @param v 64-bit value to swap.
 * @return Byte-swapped 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t
__swap64(uint64_t v) {
  return __builtin_bswap64(v);
}

/**
 * Packs an 8-bit integer (identity function).
 *
 * @param v 8-bit value.
 * @return The same 8-bit value.
 */
static inline __attribute__((always_inline, const)) uint8_t p8(uint8_t v) {
  return v;
}

/**
 * Unpacks an 8-bit integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return 8-bit value at the pointer.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint8_t
u8(const void *p) {
  return *(const uint8_t *)p;
}

/**
 * Packs a 16-bit integer as little-endian.
 *
 * @param v Host-order 16-bit value.
 * @return Little-endian 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t p16le(uint16_t v) {
  return AXIUM_IS_BIG_ENDIAN ? __swap16(v) : v;
}

/**
 * Packs a 16-bit integer as big-endian.
 *
 * @param v Host-order 16-bit value.
 * @return Big-endian 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t p16be(uint16_t v) {
  return AXIUM_IS_BIG_ENDIAN ? v : __swap16(v);
}

/**
 * Unpacks a 16-bit little-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 16-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint16_t
u16le(const void *p) {
  uint16_t v;
  memcpy(&v, p, 2);
  return p16le(v);
}

/**
 * Unpacks a 16-bit big-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 16-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint16_t
u16be(const void *p) {
  uint16_t v;
  memcpy(&v, p, 2);
  return p16be(v);
}

/**
 * Packs a 32-bit integer as little-endian.
 *
 * @param v Host-order 32-bit value.
 * @return Little-endian 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t p32le(uint32_t v) {
  return AXIUM_IS_BIG_ENDIAN ? __swap32(v) : v;
}

/**
 * Packs a 32-bit integer as big-endian.
 *
 * @param v Host-order 32-bit value.
 * @return Big-endian 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t p32be(uint32_t v) {
  return AXIUM_IS_BIG_ENDIAN ? v : __swap32(v);
}

/**
 * Unpacks a 32-bit little-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 32-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint32_t
u32le(const void *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return p32le(v);
}

/**
 * Unpacks a 32-bit big-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 32-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint32_t
u32be(const void *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return p32be(v);
}

/**
 * Packs a 64-bit integer as little-endian.
 *
 * @param v Host-order 64-bit value.
 * @return Little-endian 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t p64le(uint64_t v) {
  return AXIUM_IS_BIG_ENDIAN ? __swap64(v) : v;
}

/**
 * Packs a 64-bit integer as big-endian.
 *
 * @param v Host-order 64-bit value.
 * @return Big-endian 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t p64be(uint64_t v) {
  return AXIUM_IS_BIG_ENDIAN ? v : __swap64(v);
}

/**
 * Unpacks a 64-bit little-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 64-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint64_t
u64le(const void *p) {
  uint64_t v;
  memcpy(&v, p, 8);
  return p64le(v);
}

/**
 * Unpacks a 64-bit big-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 64-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint64_t
u64be(const void *p) {
  uint64_t v;
  memcpy(&v, p, 8);
  return p64be(v);
}

/**
 * Packs a 16-bit integer using native-endian.
 *
 * @param v Host-order 16-bit value.
 * @return Native-endian 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t p16(uint16_t v) {
  return v;
}

/**
 * Packs a 32-bit integer using native-endian.
 *
 * @param v Host-order 32-bit value.
 * @return Native-endian 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t p32(uint32_t v) {
  return v;
}

/**
 * Packs a 64-bit integer using native-endian.
 *
 * @param v Host-order 64-bit value.
 * @return Native-endian 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t p64(uint64_t v) {
  return v;
}

/**
 * Unpacks a 16-bit native-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 16-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint16_t
u16(const void *p) {
  uint16_t v;
  memcpy(&v, p, 2);
  return v;
}

/**
 * Unpacks a 32-bit native-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 32-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint32_t
u32(const void *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return v;
}

/**
 * Unpacks a 64-bit native-endian integer from memory.
 *
 * @param p Pointer to the memory location.
 * @return Host-order 64-bit value.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint64_t
u64(const void *p) {
  uint64_t v;
  memcpy(&v, p, 8);
  return v;
}

#endif // AXIUM_PACKING_H