/**
 * @file tube.h
 * @brief Communication channel abstractions (tubes) .
 */

#ifndef AXIUM_TUBE_H
#define AXIUM_TUBE_H

#include <axium/timeout.h>
#include <sys/types.h>

/**
 * @brief Representation of a communication channel (tube) .
 *
 * A tube wraps file descriptors for stdin, stdout, and stderr of a process
 * or a network connection, providing a unified interface for communication.
 */
typedef struct {
  int read_fd;  /**< File descriptor for reading from the channel (e.g., process
                   `stdout`) . */
  int write_fd; /**< File descriptor for writing to the channel (e.g., process
                   `stdin`) . */
  int stderr_fd; /**< File descriptor for reading `stderr` , or `-1` if not
                    available. */
  pid_t
      pid; /**< PID of the associated process, or `-1` for non-process tubes. */
  double timeout; /**< Default timeout for receive operations in seconds. */
} tube;

#define _TUBE_ATTR_NONNULL __attribute__((nonnull(1)))

/**
 * @brief Sends raw data to the tube.
 *
 * @param t Pointer to the tube.
 * @param data Data to send.
 * @param size Size of the data in bytes.
 * @return ssize_t Number of bytes sent, or `-1` on _error_.
 */
ssize_t send(tube *t, const void *data, size_t size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * @brief Sends raw data followed by a newline to the tube.
 *
 * @param t Pointer to the tube.
 * @param data Data to send.
 * @param size Size of the data in bytes.
 * @return ssize_t Number of bytes sent, or `-1` on _error_.
 */
ssize_t sendline(tube *t, const void *data, size_t size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * @brief Receives data from the tube into a provided buffer.
 *
 * @param t Pointer to the tube.
 * @param buf Buffer to store the received data.
 * @param size Maximum number of bytes to receive.
 * @param timeout Timeout in seconds.
 * @return ssize_t Number of bytes received, `0` on _timeout/EOF_, or `-1` on
 * _error_.
 */
ssize_t recv(tube *t, void *buf, size_t size, double timeout) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * @brief Receives data until a specific sequence is found.
 *
 * @param t Pointer to the tube.
 * @param delim The delimiter string.
 * @param timeout Timeout in seconds.
 * @param out_size Optional: Pointer to store the size of the received data (can
 * be `NULL`) .
 * @return void* Pointer to the received data (_null-terminated_, __caller must
 * free__), or `NULL` on _error_ or _timeout_.
 */
void *recvuntil(tube *t, const char *delim, double timeout,
                size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * @brief Receives one line from the tube.
 *
 * @param t Pointer to the tube.
 * @param timeout Timeout in seconds.
 * @param out_size Optional: Pointer to store the size (can be `NULL`).
 * @return void* Pointer to the received data (_null-terminated_, __caller must
 * free__), or `NULL` on _error_.
 */
void *recvline(tube *t, double timeout, size_t *out_size) _TUBE_ATTR_NONNULL;

/**
 * @brief Receives up to numlines lines from the tube.
 *
 * @param t Pointer to the tube.
 * @param numlines Maximum number of lines to receive.
 * @param timeout Timeout in seconds.
 * @param out_count Optional: Pointer to store the number of lines actually
 * received.
 * @return void** _NULL-terminated array_ of pointers to lines (__caller must
 * free each line and the array__), or `NULL` on _error_.
 */
void **recvlines(tube *t, size_t numlines, double timeout,
                 size_t *out_count) _TUBE_ATTR_NONNULL;

/**
 * @brief Receives all available data from the tube until _EOF_ and closes it.
 *
 * @param t Pointer to the tube.
 * @param timeout Timeout in seconds.
 * @param out_size Optional: Pointer to store the total size (can be `NULL`).
 * @return void* Pointer to the received data (_null-terminated_, __caller must
 * free__), or `NULL` on _error_.
 */
void *recvall(tube *t, double timeout, size_t *out_size) _TUBE_ATTR_NONNULL;

/**
 * @brief Receives until a delimiter and then sends data.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional: Pointer to store the size of the data received
 * while waiting.
 * @return void* Pointer to the data received while waiting (_null-terminated_,
 * __caller must free__), or `NULL` on _error_.
 */
void *sendafter(tube *t, const char *delim, const void *data, size_t size,
                double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * @brief Receives until a delimiter and then sends data followed by a
 * newline.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional: Pointer to store the size of the data received
 * while waiting.
 * @return void* Pointer to the data received while waiting (_null-terminated_,
 * __caller must free__), or `NULL` on _error_.
 */
void *sendlineafter(tube *t, const char *delim, const void *data, size_t size,
                    double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * @brief Sends data and then receives until a delimiter is found.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for after sending.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional: Pointer to store the size of the data received
 * after sending.
 * @return void* Pointer to the data received after sending (_null-terminated_,
 * __caller must free__), or `NULL` on _error_.
 */
void *sendthen(tube *t, const char *delim, const void *data, size_t size,
               double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * @brief Sends data + newline and then receives until a delimiter is found.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for after sending.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional: Pointer to store the size of the data received
 * after sending.
 * @return void* Pointer to the data received after sending (_null-terminated_,
 * __caller must free__), or `NULL` on _error_.
 */
void *sendlinethen(tube *t, const char *delim, const void *data, size_t size,
                   double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * @brief Switches the tube to interactive mode, connecting `stdin` and
 * `stdout` .
 *
 * @param t Pointer to the tube.
 * @param prompt Prompt string to display (if `NULL`, default red `λ` is used) .
 */
void interactive(tube *t, const char *prompt) _TUBE_ATTR_NONNULL;

/**
 * @brief Frees a buffer allocated by a tube function.
 */
void t_free(void *ptr);

/**
 * @brief Frees a _NULL-terminated array_ of lines allocated by `t_recvlines` .
 */
void t_freelines(void **lines);

/**
 * @brief Closes the tube and cleans up resources.
 */
void t_close(tube *t);

#endif // AXIUM_TUBE_H
