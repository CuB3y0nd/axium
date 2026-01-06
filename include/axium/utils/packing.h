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
 * @brief Byte-swap a 16-bit integer.
 * @param v The 16-bit value to swap.
 * @return The byte-swapped 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t
__swap16(uint16_t v) {
  return __builtin_bswap16(v);
}

/**
 * @brief Byte-swap a 32-bit integer.
 * @param v The 32-bit value to swap.
 * @return The byte-swapped 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t
__swap32(uint32_t v) {
  return __builtin_bswap32(v);
}

/**
 * @brief Byte-swap a 64-bit integer.
 * @param v The 64-bit value to swap.
 * @return The byte-swapped 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t
__swap64(uint64_t v) {
  return __builtin_bswap64(v);
}

/**
 * @brief "Pack" an 8-bit integer (identity function).
 * @param v The 8-bit value.
 * @return The same 8-bit value.
 */
static inline __attribute__((always_inline, const)) uint8_t p8(uint8_t v) {
  return v;
}

/**
 * @brief Unpack an 8-bit integer from memory.
 * @param p Pointer to the memory location.
 * @return The 8-bit value at the pointer.
 */
static inline __attribute__((always_inline, pure, nonnull(1))) uint8_t
u8(const void *p) {
  return *(const uint8_t *)p;
}

/**
 * @brief Pack a 16-bit integer as Little-Endian.
 * @param v Host-order 16-bit value.
 * @return Little-Endian 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t p16le(uint16_t v) {
  return AXIUM_IS_BIG_ENDIAN ? __swap16(v) : v;
}

/**
 * @brief Pack a 16-bit integer as Big-Endian.
 * @param v Host-order 16-bit value.
 * @return Big-Endian 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t p16be(uint16_t v) {
  return AXIUM_IS_BIG_ENDIAN ? v : __swap16(v);
}

/**
 * @brief Unpack a 16-bit Little-Endian integer from memory.
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
 * @brief Unpack a 16-bit Big-Endian integer from memory.
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
 * @brief Pack a 32-bit integer as Little-Endian.
 * @param v Host-order 32-bit value.
 * @return Little-Endian 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t p32le(uint32_t v) {
  return AXIUM_IS_BIG_ENDIAN ? __swap32(v) : v;
}

/**
 * @brief Pack a 32-bit integer as Big-Endian.
 * @param v Host-order 32-bit value.
 * @return Big-Endian 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t p32be(uint32_t v) {
  return AXIUM_IS_BIG_ENDIAN ? v : __swap32(v);
}

/**
 * @brief Unpack a 32-bit Little-Endian integer from memory.
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
 * @brief Unpack a 32-bit Big-Endian integer from memory.
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
 * @brief Pack a 64-bit integer as Little-Endian.
 * @param v Host-order 64-bit value.
 * @return Little-Endian 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t p64le(uint64_t v) {
  return AXIUM_IS_BIG_ENDIAN ? __swap64(v) : v;
}

/**
 * @brief Pack a 64-bit integer as Big-Endian.
 * @param v Host-order 64-bit value.
 * @return Big-Endian 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t p64be(uint64_t v) {
  return AXIUM_IS_BIG_ENDIAN ? v : __swap64(v);
}

/**
 * @brief Unpack a 64-bit Little-Endian integer from memory.
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
 * @brief Unpack a 64-bit Big-Endian integer from memory.
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
 * @brief Pack a 16-bit integer using Native-Endian.
 * @param v Host-order 16-bit value.
 * @return Native-Endian 16-bit value.
 */
static inline __attribute__((always_inline, const)) uint16_t p16(uint16_t v) {
  return v;
}

/**
 * @brief Pack a 32-bit integer using Native-Endian.
 * @param v Host-order 32-bit value.
 * @return Native-Endian 32-bit value.
 */
static inline __attribute__((always_inline, const)) uint32_t p32(uint32_t v) {
  return v;
}

/**
 * @brief Pack a 64-bit integer using Native-Endian.
 * @param v Host-order 64-bit value.
 * @return Native-Endian 64-bit value.
 */
static inline __attribute__((always_inline, const)) uint64_t p64(uint64_t v) {
  return v;
}

/**
 * @brief Unpack a 16-bit Native-Endian integer from memory.
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
 * @brief Unpack a 32-bit Native-Endian integer from memory.
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
 * @brief Unpack a 64-bit Native-Endian integer from memory.
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
