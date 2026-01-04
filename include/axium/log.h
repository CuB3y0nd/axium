#ifndef AXIUM_LOG_H
#define AXIUM_LOG_H

#include <stdbool.h>

typedef enum { DEBUG = 0, INFO, WARNING, ERROR, CRITICAL } log_level_t;

/** @brief Sets the global log level. Logs below this level will not be printed.
 */
void set_log_level(log_level_t level);

/** @brief Returns the current global log level. */
log_level_t get_log_level(void);

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_BLUE "\033[1;34m"
#define ANSI_BOLD_YELLOW "\033[1;33m"
#define ANSI_ON_RED "\033[41m"
#define ANSI_WHITE "\033[37m"

#define _LOG_ATTR                                                              \
  __attribute__((format(printf, 1, 2))) __attribute__((nonnull(1)))
#define _LOG_COLD __attribute__((cold))

/** @brief Logs a success message. */
_LOG_ATTR void log_success(const char *fmt, ...);
/** @brief Logs a failure message. */
_LOG_ATTR void log_failure(const char *fmt, ...);
/** @brief Logs an informational message. */
_LOG_ATTR void log_info(const char *fmt, ...);
/** @brief Logs a warning message. */
_LOG_ATTR _LOG_COLD void log_warning(const char *fmt, ...);
/** @brief Logs an error message. */
_LOG_ATTR _LOG_COLD void log_error(const char *fmt, ...);
/** @brief Logs an exception message with [ERROR] prefix and appends the system
 * error from errno. */
_LOG_ATTR _LOG_COLD void log_exception(const char *fmt, ...);
/** @brief Logs a critical message. */
_LOG_ATTR _LOG_COLD void log_critical(const char *fmt, ...);
/** @brief Logs a debug message, visible only if log level is DEBUG. */
_LOG_ATTR void log_debug(const char *fmt, ...);
/** @brief Logs a status message. */
_LOG_ATTR void log_status(const char *fmt, ...);

/** @brief Logs an informational message only once at the given call site. */
#define log_info_once(fmt, ...)                                                \
  do {                                                                         \
    static bool _log_once = false;                                             \
    if (!_log_once) {                                                          \
      log_info(fmt, ##__VA_ARGS__);                                            \
      _log_once = true;                                                        \
    }                                                                          \
  } while (0)

/** @brief Logs a warning message only once at the given call site. */
#define log_warning_once(fmt, ...)                                             \
  do {                                                                         \
    static bool _log_once = false;                                             \
    if (!_log_once) {                                                          \
      log_warning(fmt, ##__VA_ARGS__);                                         \
      _log_once = true;                                                        \
    }                                                                          \
  } while (0)

#endif /* AXIUM_LOG_H */
