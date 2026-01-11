#ifndef AXIUM_SHELLCRAFT_COMMON_H
#define AXIUM_SHELLCRAFT_COMMON_H

#include <axium/utils/payload.h>

/** @brief Stringify a macro argument. */
#define STR(x) #x
/** @brief Expand and stringify a macro argument. */
#define XSTR(x) STR(x)

/** @brief Attribute for naked functions (no prologue/epilogue) */
#define NAKED __attribute__((naked))

/**
 * @brief Define shellcode boundaries for a naked function.
 * @param name Function name.
 */
#define DEFINE_SHELLCODE(name) NAKED void name(void)

/* clang-format off */
/**
 * @brief Inline assembly markers for the start of shellcode.
 * @param name Function name.
 */
#define SHELLCODE_START(name)                                                  \
  __asm__ volatile(                                                            \
    ".intel_syntax noprefix\n"                                                 \
    ".global " STR(name) "_start\n"                                            \
    ".global " STR(name) "_end\n" STR(name) "_start:\n"                        \
  )

/**
 * @brief Inline assembly markers for the end of shellcode.
 * @param name Function name.
 */
#define SHELLCODE_END(name)                                                    \
  __asm__ volatile(                                                            \
    STR(name) "_end:\n"                                                        \
    ".att_syntax\n"                                                            \
  )
/* clang-format on */

/**
 * @brief Append shellcode defined via DEFINE_SHELLCODE to a payload.
 * @param p Pointer to the payload.
 * @param name Name of the shellcode function.
 */
#define PAYLOAD_PUSH_SC(p, name)                                               \
  do {                                                                         \
    extern char name##_start[];                                                \
    extern char name##_end[];                                                  \
    payload_push((p), name##_start, (size_t)(name##_end - name##_start));      \
  } while (0)

/**
 * @brief Generate a unique marker for shellcode templates.
 * @param type The type of the value (uint32_t or uint64_t).
 * @param id A unique identifier.
 */
#define SC_M_uint64_t(id) (0xCAFEBABE00000000 | (id))
#define SC_M_uint32_t(id) (0x13370000 | (id))
#define SC_M(type, id) SC_M_##type(id)

/**
 * @brief Fixup a marker in the payload with a concrete value.
 *
 * The marker size is automatically determined by the type of 'val'.
 *
 * @param p Pointer to the payload.
 * @param id The marker ID used in the shellcode.
 * @param val The value to replace the marker with.
 */
#define sc_fix(p, id, val)                                                     \
  _Generic((val),                                                              \
      uint32_t: payload_patch_u32(p, SC_M(uint32_t, id), (uint32_t)(val)),     \
      uint64_t: payload_patch_u64(p, SC_M(uint64_t, id), (uint64_t)(val)))

#endif // AXIUM_SHELLCRAFT_COMMON_H
