#include <axium/log.h>
#include <axium/timeout.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static log_level_t current_log_level = INFO;
static bool is_status_active = false;
static double last_status_time = 0;
#define STATUS_THROTTLE_RATE 0.1

void set_log_level(log_level_t level) { current_log_level = level; }

log_level_t get_log_level(void) { return current_log_level; }

static void vlog(log_level_t level, const char *prefix, const char *color,
                 bool is_status, const char *fmt, va_list args) {
  if (level < current_log_level) {
    return;
  }

  if (is_status_active) {
    fprintf(stderr, "\r\x1b[K"); // Move to start of line and clear it
    is_status_active = false;
  }

  if (color) {
    fprintf(stderr, "[%s%s%s] ", color, prefix, ANSI_RESET);
  } else {
    fprintf(stderr, "[%s] ", prefix);
  }

  vfprintf(stderr, fmt, args);

  if (is_status) {
    is_status_active = true;
    fflush(stderr);
  } else {
    fprintf(stderr, "\n");
  }
}

void log_success(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "+", ANSI_BOLD_GREEN, false, fmt, args);
  va_end(args);
}

void log_failure(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "-", ANSI_BOLD_RED, false, fmt, args);
  va_end(args);
}

void log_info(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(INFO, "*", ANSI_BOLD_BLUE, false, fmt, args);
  va_end(args);
}

void log_warning(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(WARNING, "!", ANSI_BOLD_YELLOW, false, fmt, args);
  va_end(args);
}

void log_error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(ERROR, "ERROR", ANSI_ON_RED ANSI_WHITE, false, fmt, args);
  va_end(args);
  exit(EXIT_FAILURE);
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
  vlog(CRITICAL, "CRITICAL", ANSI_ON_RED ANSI_WHITE, false, fmt, args);
  va_end(args);
}

void log_debug(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(DEBUG, "DEBUG", ANSI_BOLD_RED, false, fmt, args);
  va_end(args);
}

void log_status(const char *fmt, ...) {
  double now = timeout_now();
  if (is_status_active && now - last_status_time < STATUS_THROTTLE_RATE) {
    return;
  }
  last_status_time = now;

  va_list args;
  va_start(args, fmt);
  vlog(INFO, "x", ANSI_MAGENTA, true, fmt, args);
  va_end(args);
}
