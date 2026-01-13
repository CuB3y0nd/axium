#ifndef AXIUM_SHELLCRAFT_EXT_TEMPLATE_H
#define AXIUM_SHELLCRAFT_EXT_TEMPLATE_H

#include <axium/shellcraft/common.h>

/**
 * @file template.h
 * @brief Reference template for user-defined shellcode snippets.
 *
 * Instructions:
 * 1. Copy this file to this directory and rename it (e.g., `my_snippets.h`) .
 * 2. Use `static DEFINE_SHELLCODE` to define the assembly template.
 * 3. Use `SC_M(type, id)` as placeholders for values to be patched.
 * 4. Provide a `static inline` helper function that utilizes `PAYLOAD_PUSH_SC`
 *    and `sc_fix` .
 */

#if defined(__x86_64__)

/* --- 1. Shellcode Template Definition --- */

/* 'static' ensures the snippet is local to the translation unit to avoid
 * conflicts */

/* clang-format off */
static DEFINE_SHELLCODE(usc_template_sc) {
  SHELLCODE_START(usc_template_sc);
  __asm__ volatile(
      "mov rax, " XSTR(SC_M(uint64_t, 1)) "\n" // Placeholder 1: Target address
      "mov rdi, " XSTR(SC_M(uint64_t, 2)) "\n" // Placeholder 2: First argument
      "call rax\n"
      "ret\n");
  SHELLCODE_END(usc_template_sc);
}
/* clang-format on */

/* --- 2. Helper Function --- */

/**
 * @brief Example template helper function.
 * @param p Pointer to the payload.
 * @param target Target function address.
 * @param arg Argument for the function.
 */
static inline void usc_template(payload_t *p, uint64_t target, uint64_t arg) {
  PAYLOAD_PUSH_SC(p, usc_template_sc);
  sc_fix(p, 1, target); // Patch placeholder 1
  sc_fix(p, 2, arg);    // Patch placeholder 2
}

#endif // __x86_64__

#endif // AXIUM_SHELLCRAFT_EXT_TEMPLATE_H
