/**
 * @file process.h
 * @brief Process spawning and tube creation.
 */

#ifndef AXIUM_PROCESS_H
#define AXIUM_PROCESS_H

#include <axium/tubes/tube.h>
#include <unistd.h>

/**
 * @brief Flags for tube redirection.
 */
typedef enum {
  TUBE_STDIN = 1 << 0,  /**< Redirect `stdin` . */
  TUBE_STDOUT = 1 << 1, /**< Redirect `stdout` . */
  TUBE_STDERR = 1 << 2, /**< Redirect `stderr` . */
  TUBE_ALL =
      TUBE_STDIN | TUBE_STDOUT | TUBE_STDERR /**< Redirect all streams. */
} tube_flags_t;

/**
 * @brief Spawns a new process with custom redirection.
 *
 * @param argv Argument vector (_null-terminated_) .
 * @param envp Environment vector (_null-terminated_) .
 * @param flags Redirection flags specifying which streams to pipe.
 * @return tube* Pointer to the created tube, or `NULL` on _failure_ .
 */
__attribute__((warn_unused_result)) tube *
process_ext(char *const argv[], char *const envp[], tube_flags_t flags);

/**
 * @brief Spawns a new process and returns a tube for communication.
 *
 * By default, this redirects `stdin`, `stdout`, and `stderr` .
 *
 * @param argv Argument vector (_null-terminated_) .
 * @param envp Environment vector (_null-terminated_) .
 * @return tube* Pointer to the created tube, or `NULL` on _failure_ .
 */
__attribute__((warn_unused_result)) tube *process(char *const argv[],
                                                  char *const envp[]);

/**
 * @brief Closes all pipes, waits for the child process to exit, and frees the
 * tube.
 * @param t Pointer to the tube struct.
 */
void t_close(tube *t) __attribute__((nonnull(1)));

/** @brief Returns the _pid_ of the created process. */
static inline pid_t t_pid(tube *t) { return t ? t->pid : -1; }

/** @brief Returns the _file descriptor_ for the child's `stdin` . */
static inline int t_in(tube *t) { return t ? t->write_fd : -1; }

/** @brief Returns the _file descriptor_ for the child's `stdout` . */
static inline int t_out(tube *t) { return t ? t->read_fd : -1; }

/** @brief Returns the _file descriptor_ for the child's `stderr` . */
static inline int t_err(tube *t) { return t ? t->stderr_fd : -1; }

/** @brief Closes the child's `stdin` pipe. Useful for signaling `EOF` . */
static inline void t_cin(tube *t) {
  if (t && t->write_fd != -1) {
    close(t->write_fd);
    t->write_fd = -1;
  }
}

/** @brief Closes the child's `stdout` pipe. */
static inline void t_cout(tube *t) {
  if (t && t->read_fd != -1) {
    close(t->read_fd);
    t->read_fd = -1;
  }
}

/** @brief Closes the child's `stderr` pipe. */
static inline void t_cerr(tube *t) {
  if (t && t->stderr_fd != -1) {
    close(t->stderr_fd);
    t->stderr_fd = -1;
  }
}

#endif // AXIUM_PROCESS_H
