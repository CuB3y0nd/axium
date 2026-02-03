/**
 * @file snippets.h
 *
 * Provides pre-defined shellcode snippets for common exploitation tasks.
 */

#ifndef AXIUM_SHELLCRAFT_SNIPPETS_H
#define AXIUM_SHELLCRAFT_SNIPPETS_H

#include <axium/shellcraft/common.h>
#include <axium/utils/payload.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup sc_snippets Shellcode Snippets
 *
 * Common shellcode snippets.
 * @{
 */

/**
 * Kernel: Escalates privileges via `commit_creds(prepare_kernel_cred(0))`.
 *
 * @param p Pointer to the payload.
 * @param pkc Address of prepare_kernel_cred.
 * @param cc Address of commit_creds.
 */
void ksc_escalate(payload_t *p, uint64_t pkc, uint64_t cc);

/**
 * Kernel: Disables seccomp for the current task by clearing the TIF_SECCOMP bit
 * at a given offset of the `current_task` structure.
 *
 * @param p Pointer to the payload.
 * @param task_struct_offset Offset of the `current_task` structure.
 * @param tif_seccomp_bit Bit index to clear (not a mask).
 */
void ksc_unseccomp(payload_t *p, uint64_t task_struct_offset,
                   uint64_t tif_seccomp_bit);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // AXIUM_SHELLCRAFT_SNIPPETS_H
