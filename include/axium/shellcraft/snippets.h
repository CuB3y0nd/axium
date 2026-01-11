#ifndef AXIUM_SHELLCRAFT_SNIPPETS_H
#define AXIUM_SHELLCRAFT_SNIPPETS_H

#include <axium/shellcraft/common.h>
#include <axium/utils/payload.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup sc_snippets Shellcode Snippets
 * @{
 */

/**
 * @brief Kernel: Escalate privileges via commit_creds(prepare_kernel_cred(0)).
 *
 * Context: x86_64 Kernel.
 *
 * @param p Pointer to the payload.
 * @param pkc Address of prepare_kernel_cred.
 * @param cc Address of commit_creds.
 */
void ksc_escalate(payload_t *p, uint64_t pkc, uint64_t cc);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // AXIUM_SHELLCRAFT_SNIPPETS_H
