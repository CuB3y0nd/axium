#include <axium/log.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static log_level_t current_log_level = INFO;

void set_log_level(log_level_t level) { current_log_level = level; }

log_level_t get_log_level(void) { return current_log_level; }

static void vlog(log_level_t level, const char *prefix, const char *color,
                 const char *fmt, va_list args) {
  if (level < current_log_level) {
    return;
  }

  if (color) {
    fprintf(stderr, "[%s%s%s] ", color, prefix, ANSI_RESET);
  } else {
    fprintf(stderr, "[%s] ", prefix);
  }
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
}

void log_success(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "+", ANSI_BOLD_GREEN, fmt, args);
  va_end(args);
}

void log_failure(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "-", ANSI_BOLD_RED, fmt, args);
  va_end(args);
}

void log_info(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "*", ANSI_BOLD_BLUE, fmt, args);
  va_end(args);
}

void log_warning(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(WARNING, "!", ANSI_BOLD_YELLOW, fmt, args);
  va_end(args);
}

void log_error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(ERROR, "ERROR", ANSI_ON_RED ANSI_WHITE, fmt, args);
  va_end(args);
}

void log_exception(const char *fmt, ...) {
  char buf[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  log_error("%s: %s", buf, strerror(errno));
}

void log_critical(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(CRITICAL, "CRITICAL", ANSI_ON_RED ANSI_WHITE, fmt, args);
  va_end(args);
}

void log_debug(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(DEBUG, "DEBUG", ANSI_BOLD_RED, fmt, args);
  va_end(args);
}

void log_status(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "x", ANSI_MAGENTA, fmt, args);
  va_end(args);
}
