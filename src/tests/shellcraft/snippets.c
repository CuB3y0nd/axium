#include "../test_common.h"
#include <axium/axium.h>
#include <axium/shellcraft/ext/template.h>

// Helper to find a 64-bit value in a non-aligned buffer
static bool find_u64(const payload_t *p, uint64_t target) {
  if (p->size < 8)
    return false;
  for (size_t i = 0; i <= p->size - 8; i++) {
    // Read 8 bytes at current offset and compare
    uint64_t val;
    memcpy(&val, p->data + i, 8);
    if (val == target)
      return true;
  }
  return false;
}

int main() {
  payload_t p;

#if defined(__x86_64__)
  /* --- 1. Test Official Snippets (ksc_escalate) --- */
  payload_init(&p);
  ksc_escalate(&p, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL);
  ASSERT_TRUE(find_u64(&p, 0x1122334455667788ULL),
              "Official ksc_escalate: PKC found");
  ASSERT_TRUE(find_u64(&p, 0x99aabbccddeeff00ULL),
              "Official ksc_escalate: CC found");
  ASSERT_EQ(p.size, 28, "Official ksc_escalate: Size 28 bytes");
  payload_fini(&p);

  /* --- 2. Test Custom Extensions (usc_template) --- */
  payload_init(&p);
  usc_template(&p, 0xdeadbeefULL, 0x1337ULL);
  ASSERT_TRUE(find_u64(&p, 0xdeadbeefULL), "Custom usc_template: Target found");
  ASSERT_TRUE(find_u64(&p, 0x1337ULL), "Custom usc_template: Arg found");
  ASSERT_EQ(p.size, 23, "Custom usc_template: Size 23 bytes");
  payload_fini(&p);

  /* --- 3. Test sc_fix Global Impact --- */
  payload_init(&p);

  // Push raw shellcodes without patching markers through helpers
  PAYLOAD_PUSH_SC(&p, ksc_escalate_sc);
  PAYLOAD_PUSH_SC(&p, usc_template_sc);

  // Initially, markers SC_M(1) and SC_M(2) exist in multiple places
  uint64_t m1 = SC_M(1);
  uint64_t m2 = SC_M(2);
  ASSERT_TRUE(find_u64(&p, m1), "Marker 1 exists before fix");
  ASSERT_TRUE(find_u64(&p, m2), "Marker 2 exists before fix");

  // Fix marker 1 globally
  uint64_t val1 = 0xAAAAAAAAAAAAAAAAULL;
  sc_fix(&p, 1, val1);

  ASSERT_TRUE(find_u64(&p, val1), "sc_fix: New value 1 applied");
  ASSERT_TRUE(!find_u64(&p, m1), "sc_fix: Old marker 1 removed");

  // Fix marker 2 globally
  uint64_t val2 = 0xBBBBBBBBBBBBBBBBULL;
  sc_fix(&p, 2, val2);

  ASSERT_TRUE(find_u64(&p, val2), "sc_fix: New value 2 applied");
  ASSERT_TRUE(!find_u64(&p, m2), "sc_fix: Old marker 2 removed");

  payload_fini(&p);
#else
  (void)p;
  (void)ksc_escalate_sc; // Suppress unused
  (void)usc_template_sc;
#endif

  return 0;
}
