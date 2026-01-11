#ifndef AXIUM_H
#define AXIUM_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* Common standard headers */
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Axium modules */
#include <axium/log.h>
#include <axium/shellcraft/common.h>
#include <axium/shellcraft/snippets.h>
#include <axium/timeout.h>
#include <axium/tubes/process.h>
#include <axium/tubes/tube.h>
#include <axium/utils/fiddling.h>
#include <axium/utils/packing.h>
#include <axium/utils/payload.h>
#include <axium/utils/proc.h>

#ifdef __cplusplus
}
#endif

#endif /* AXIUM_H */
