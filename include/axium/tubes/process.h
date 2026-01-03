#ifndef AXIUM_PROCESS_H
#define AXIUM_PROCESS_H

#include <sys/types.h>
#include <unistd.h>

typedef struct {
  int read_fd;   // Read from process stdout
  int write_fd;  // Write to process stdin
  int stderr_fd; // Read from process stderr
  pid_t pid;
} tube;

/**
 * @brief Spawns a new process and returns a tube for communication.
 *
 * @param argv Null-terminated array of strings for the command and arguments.
 * @param envp Optional null-terminated array of environment variables.
 * @return A pointer to a tube struct on success, NULL on failure.
 */
__attribute__((warn_unused_result)) tube *process(char *const argv[],
                                                  char *const envp[])
    __attribute__((nonnull(1)));

/**
 * @brief Closes all pipes, waits for the child process to exit, and frees the
 * tube.
 * @param t Pointer to the tube struct.
 */
void t_close(tube *t) __attribute__((nonnull(1)));

/** @brief Returns the file descriptor for the child's stdin. */
static inline int t_in(tube *t) { return t ? t->write_fd : -1; }

/** @brief Returns the file descriptor for the child's stdout. */
static inline int t_out(tube *t) { return t ? t->read_fd : -1; }

/** @brief Returns the file descriptor for the child's stderr. */
static inline int t_err(tube *t) { return t ? t->stderr_fd : -1; }

/** @brief Closes the child's stdin pipe. Useful for signaling EOF. */
static inline void t_cin(tube *t) {
  if (t && t->write_fd != -1) {
    close(t->write_fd);
    t->write_fd = -1;
  }
}

/** @brief Closes the child's stdout pipe. */
static inline void t_cout(tube *t) {
  if (t && t->read_fd != -1) {
    close(t->read_fd);
    t->read_fd = -1;
  }
}

/** @brief Closes the child's stderr pipe. */
static inline void t_cerr(tube *t) {
  if (t && t->stderr_fd != -1) {
    close(t->stderr_fd);
    t->stderr_fd = -1;
  }
}

#endif // AXIUM_PROCESS_H
