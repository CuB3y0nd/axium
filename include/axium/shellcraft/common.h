#ifndef AXIUM_SHELLCRAFT_COMMON_H
#define AXIUM_SHELLCRAFT_COMMON_H

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
#define DEFINE_SHELLCODE(name)                                                 \
  extern char name##_start[];                                                  \
  extern char name##_end[];                                                    \
  NAKED void name(void)

/**
 * @brief Inline assembly markers for the start of shellcode.
 * @param name Function name.
 */
#define SHELLCODE_START(name)                                                  \
  __asm__ volatile(".intel_syntax noprefix\n"                                  \
                   ".global " #name "_start\n"                                 \
                   ".global " #name "_end\n" #name "_start:\n")

/**
 * @brief Inline assembly markers for the end of shellcode.
 * @param name Function name.
 */
#define SHELLCODE_END(name)                                                    \
  __asm__ volatile(#name "_end:\n"                                             \
                         ".att_syntax\n")

#include <axium/utils/payload.h>

/**
 * @brief Append shellcode defined via DEFINE_SHELLCODE to a payload.
 * @param p Pointer to the payload.
 * @param name Name of the shellcode function.
 */
#define PAYLOAD_PUSH_SC(p, name)                                               \
  payload_push(p, &name##_start, (size_t)(name##_end - name##_start))

#endif // AXIUM_SHELLCRAFT_COMMON_H
