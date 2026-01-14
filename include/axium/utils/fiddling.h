/**
 * @file fiddling.h
 *
 * Provides miscellaneous utilities, including hexdump.
 */

#ifndef AXIUM_FIDDLING_H
#define AXIUM_FIDDLING_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Theme configuration for hexdump colors.
 *
 * All values should be ANSI escape codes. Empty strings can be used to
 * disable coloring for a specific category.
 */
typedef struct {
  const char *null;          /**< Color for `0x00` and `0x0a`. */
  const char *printable;     /**< Color for printable ASCII characters. */
  const char *non_printable; /**< Color for other non-printable characters. */
  const char *ff;            /**< Color for `0xff` bytes. */
  const char *marker;        /**< Color for group separators (`│`). */
  const char *reset;         /**< Reset escape code (for example, `\x1b[0m`). */
} hexdump_theme;

#define HEXDUMP_DEFAULT_THEME                                                  \
  {.null = "\x1b[31m",                                                         \
   .printable = "",                                                            \
   .non_printable = "\x1b[34m",                                                \
   .ff = "\x1b[32m",                                                           \
   .marker = "\x1b[34m",                                                       \
   .reset = "\x1b[0m"}

/** Configuration options for the hexdump function. */
typedef struct {
  const char *prefix; /**< String to prepend to each line of output. */
  size_t width;       /**< Number of bytes to display per line. */
  size_t groupsize;   /**< Number of bytes per group (space separated). */
  bool skip;    /**< If true, repeated identical lines are replaced by `*`. */
  size_t begin; /**< Initial offset value to display in the left column. */
  bool total;   /**< If true, print total bytes at the end. */
  bool color;   /**< If true, enable colored output using the theme. */
  hexdump_theme theme; /**< Theme configuration for colors. */
} hexdump_options;

#define HEXDUMP_DEFAULT_OPTIONS                                                \
  {.prefix = "",                                                               \
   .width = 16,                                                                \
   .groupsize = 4,                                                             \
   .skip = true,                                                               \
   .begin = 0,                                                                 \
   .total = true,                                                              \
   .color = true,                                                              \
   .theme = HEXDUMP_DEFAULT_THEME}

/**
 * Performs a hexdump of the provided data with various options.
 *
 * This function outputs a formatted hexdump to `stdout`, including offset,
 * hex representation, and ASCII representation. It supports color themes,
 * grouping, and repeated line skipping.
 *
 * @param data Data to dump.
 * @param size Size of the data in bytes.
 * @param options Pointer to options struct for customization. If `NULL`,
 *     `HEXDUMP_DEFAULT_OPTIONS` is used.
 */
void hexdump(const void *data, size_t size, const hexdump_options *options)
    __attribute__((nonnull(1))) __attribute__((cold));

#endif // AXIUM_FIDDLING_H
