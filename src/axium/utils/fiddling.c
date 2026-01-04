#include <axium/utils/fiddling.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const char HEX_CHARS[] = "0123456789abcdef";

/**
 * @brief Fast string append function.
 *
 * @param dst Destination buffer to append to.
 * @param src Source string to append.
 * @return char* Pointer to the end of the appended string in dst.
 */
static inline char *fast_append(char *dst, const char *src) {
  if (!src)
    return dst;
  while (*src)
    *dst++ = *src++;
  return dst;
}

/**
 * @brief Formats a single byte into hex and ASCII buffers with optional
 * coloring.
 *
 * @param c The byte to format.
 * @param color Whether to use ANSI color codes.
 * @param theme The color theme to use.
 * @param h_ptr Pointer to the current position in the hex output buffer.
 * @param a_ptr Pointer to the current position in the ASCII output buffer.
 */
static inline void format_byte(unsigned char c, bool color,
                               const hexdump_theme *theme, char **h_ptr,
                               char **a_ptr) {
  const char *c_code = NULL;
  bool printable = isprint(c);

  if (color) {
    if (c == 0x00 || c == 0x0a)
      c_code = theme->null;
    else if (c == 0xff)
      c_code = theme->ff;
    else if (!printable)
      c_code = theme->non_printable;
    else if (theme->printable && theme->printable[0] != '\0')
      c_code = theme->printable;
  }

  if (c_code && c_code[0] != '\0') {
    /* Hex part with color */
    *h_ptr = fast_append(*h_ptr, c_code);
    *(*h_ptr)++ = HEX_CHARS[(c >> 4) & 0xF];
    *(*h_ptr)++ = HEX_CHARS[c & 0xF];
    *h_ptr = fast_append(*h_ptr, theme->reset);
    *(*h_ptr)++ = ' ';

    /* ASCII part with color */
    *a_ptr = fast_append(*a_ptr, c_code);
    *(*a_ptr)++ = printable ? c : '.';
    *a_ptr = fast_append(*a_ptr, theme->reset);
  } else {
    /* Hex part no color */
    *(*h_ptr)++ = HEX_CHARS[(c >> 4) & 0xF];
    *(*h_ptr)++ = HEX_CHARS[c & 0xF];
    *(*h_ptr)++ = ' ';

    /* ASCII part no color */
    *(*a_ptr)++ = printable ? c : '.';
  }
}

void hexdump(const void *data, size_t size, const hexdump_options *options) {
  hexdump_options opt =
      options ? *options : (hexdump_options)HEXDUMP_DEFAULT_OPTIONS;
  if (opt.width == 0)
    opt.width = 16;
  if (opt.groupsize == 0)
    opt.groupsize = 4;

  const unsigned char *p = (const unsigned char *)data;
  const unsigned char *last_chunk = NULL;
  bool skipping = false;
  size_t numb = 0;

  char hex_buf[4096];
  char ascii_buf[4096];

  while (numb < size) {
    size_t chunk = (size - numb > opt.width) ? opt.width : (size - numb);

    if (opt.skip && last_chunk && chunk == opt.width) {
      if (memcmp(p + numb, last_chunk, opt.width) == 0) {
        if (!skipping) {
          printf("%s*\n", opt.prefix ? opt.prefix : "");
          skipping = true;
        }
        numb += chunk;
        continue;
      }
    }

    skipping = false;
    last_chunk = p + numb;

    printf("%s%08zx  ", opt.prefix ? opt.prefix : "", opt.begin + numb);

    char *h_ptr = hex_buf;
    char *a_ptr = ascii_buf;

    for (size_t i = 0; i < opt.width; i++) {
      if (i < chunk) {
        format_byte(p[numb + i], opt.color, &opt.theme, &h_ptr, &a_ptr);

        if ((i + 1) % opt.groupsize == 0 && i < opt.width - 1) {
          *h_ptr++ = ' ';
          /* Only add separator to ASCII if there's more data to come in this
           * line */
          if (i + 1 < chunk) {
            if (opt.color && opt.theme.marker && opt.theme.marker[0] != '\0') {
              a_ptr = fast_append(a_ptr, opt.theme.marker);
              a_ptr = fast_append(a_ptr, "│");
              a_ptr = fast_append(a_ptr, opt.theme.reset);
            } else {
              a_ptr = fast_append(a_ptr, "│");
            }
          }
        }
      } else {
        /* Padding for hex part to maintain vertical alignment of the ASCII box
         */
        *h_ptr++ = ' ';
        *h_ptr++ = ' ';
        *h_ptr++ = ' ';
        if ((i + 1) % opt.groupsize == 0 && i < opt.width - 1) {
          *h_ptr++ = ' ';
        }
      }
    }

    *h_ptr = '\0';
    *a_ptr = '\0';
    printf("%s │%s│\n", hex_buf, ascii_buf);
    numb += chunk;
  }

  if (opt.total) {
    printf("%s%08zx\n", opt.prefix ? opt.prefix : "", opt.begin + size);
  }
}
