/**
 * @file tube.h
 *
 * Provides communication channel abstractions (tubes).
 */

#ifndef AXIUM_TUBE_H
#define AXIUM_TUBE_H

#include <axium/timeout.h>
#include <sys/types.h>

/**
 * Representation of a communication channel (tube).
 *
 * A tube wraps file descriptors for `stdin`, `stdout`, and `stderr` of a
 * process or a network connection, providing a unified interface for
 * communication.
 */
typedef struct {
  int read_fd;   /**< File descriptor for reading from the channel. */
  int write_fd;  /**< File descriptor for writing to the channel. */
  int stderr_fd; /**< File descriptor for reading `stderr`, or `-1` if
                    unavailable. */
  pid_t
      pid; /**< PID of the associated process, or `-1` for non-process tubes. */
  double timeout; /**< Default timeout for receive operations in seconds. */
} tube;

#define _TUBE_ATTR_NONNULL __attribute__((nonnull(1)))

/**
 * Sends raw data to the tube.
 *
 * @param t Pointer to the tube.
 * @param data Data to send.
 * @param size Size of the data in bytes.
 * @return Number of bytes sent, or `-1` on error.
 */
ssize_t send(tube *t, const void *data, size_t size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * Sends raw data followed by a newline to the tube.
 *
 * @param t Pointer to the tube.
 * @param data Data to send.
 * @param size Size of the data in bytes.
 * @return Number of bytes sent, or `-1` on error.
 */
ssize_t sendline(tube *t, const void *data, size_t size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * Receives data from the tube into a provided buffer.
 *
 * @param t Pointer to the tube.
 * @param buf Buffer to store the received data.
 * @param size Maximum number of bytes to receive.
 * @param timeout Timeout in seconds.
 * @return Number of bytes received, `0` on timeout/`EOF`, or `-1` on error.
 */
ssize_t recv(tube *t, void *buf, size_t size, double timeout) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * Receives data until a specific sequence is found.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string.
 * @param timeout Timeout in seconds.
 * @param out_size Optional pointer to store the size of the received data.
 * @return Pointer to the received data, or `NULL` on error or timeout. The
 *     caller is responsible for freeing the returned buffer.
 */
void *recvuntil(tube *t, const char *delim, double timeout,
                size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * Receives one line from the tube.
 *
 * @param t Pointer to the tube.
 * @param timeout Timeout in seconds.
 * @param out_size Optional pointer to store the size.
 * @return Pointer to the received data, or `NULL` on error. The caller is
 *     responsible for freeing the returned buffer.
 */
void *recvline(tube *t, double timeout, size_t *out_size) _TUBE_ATTR_NONNULL;

/**
 * Receives multiple lines from the tube.
 *
 * @param t Pointer to the tube.
 * @param numlines Maximum number of lines to receive.
 * @param timeout Timeout in seconds.
 * @param out_count Optional pointer to store the number of lines received.
 * @return `NULL`-terminated array of pointers to lines, or `NULL` on error. The
 *     caller is responsible for freeing each line and the array.
 */
void **recvlines(tube *t, size_t numlines, double timeout,
                 size_t *out_count) _TUBE_ATTR_NONNULL;

/**
 * Receives all available data from the tube until `EOF` and closes it.
 *
 * @param t Pointer to the tube.
 * @param timeout Timeout in seconds.
 * @param out_size Optional pointer to store the total size.
 * @return Pointer to the received data, or `NULL` on error. The caller is
 *     responsible for freeing the returned buffer.
 */
void *recvall(tube *t, double timeout, size_t *out_size) _TUBE_ATTR_NONNULL;

/**
 * Receives until a delimiter and then sends data.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional pointer to store the size of the received data.
 * @return Pointer to the data received while waiting, or `NULL` on error. The
 *     caller is responsible for freeing the returned buffer.
 */
void *sendafter(tube *t, const char *delim, const void *data, size_t size,
                double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * Receives until a delimiter and then sends data followed by a newline.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional pointer to store the size of the received data.
 * @return Pointer to the data received while waiting, or `NULL` on error. The
 *     caller is responsible for freeing the returned buffer.
 */
void *sendlineafter(tube *t, const char *delim, const void *data, size_t size,
                    double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * Sends data and then receives until a delimiter is found.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for after sending.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional pointer to store the size of the received data.
 * @return Pointer to the data received after sending, or `NULL` on error. The
 *     caller is responsible for freeing the returned buffer.
 */
void *sendthen(tube *t, const char *delim, const void *data, size_t size,
               double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * Sends data and a newline, then receives until a delimiter is found.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for after sending.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional pointer to store the size of the received data.
 * @return Pointer to the data received after sending, or `NULL` on error. The
 *     caller is responsible for freeing the returned buffer.
 */
void *sendlinethen(tube *t, const char *delim, const void *data, size_t size,
                   double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * Switches the tube to interactive mode, connecting `stdin` and `stdout`.
 *
 * @param t Pointer to the tube.
 * @param prompt Prompt string to display. If `NULL`, the default is used.
 */
void interactive(tube *t, const char *prompt) _TUBE_ATTR_NONNULL;

/**
 * Frees a buffer allocated by a tube function.
 *
 * @param ptr Pointer to the buffer to free.
 */
void t_free(void *ptr);

/**
 * Frees a `NULL`-terminated array of lines allocated by `recvlines`.
 *
 * @param lines Array of lines to free.
 */
void t_freelines(void **lines);

/**
 * Closes the tube and cleans up resources.
 *
 * @param t Pointer to the tube.
 */
void t_close(tube *t) _TUBE_ATTR_NONNULL;

#undef _TUBE_ATTR_NONNULL

#endif // AXIUM_TUBE_H
