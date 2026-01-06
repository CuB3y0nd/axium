#ifndef AXIUM_TUBE_H
#define AXIUM_TUBE_H

#include <axium/timeout.h>
#include <sys/types.h>

/**
 * @brief Representation of a communication channel (tube).
 */
typedef struct {
  int read_fd;    /**< Read from process stdout. */
  int write_fd;   /**< Write to process stdin. */
  int stderr_fd;  /**< Read from process stderr. */
  pid_t pid;      /**< PID of the process associated with the tube. */
  double timeout; /**< Default timeout. */
} tube;

#define _TUBE_ATTR_NONNULL __attribute__((nonnull(1)))

/**
 * @brief Sends raw data to the tube.
 *
 * @param t Pointer to the tube.
 * @param data Data to send.
 * @param size Size of the data in bytes.
 * @return ssize_t Number of bytes sent, or -1 on error.
 */
ssize_t send(tube *t, const void *data, size_t size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * @brief Sends raw data followed by a newline to the tube.
 *
 * @param t Pointer to the tube.
 * @param data Data to send.
 * @param size Size of the data in bytes.
 * @return ssize_t Number of bytes sent, or -1 on error.
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
 * @return ssize_t Number of bytes received, 0 on timeout/EOF, or -1 on error.
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
 * be NULL).
 * @return void* Pointer to the received data (null-terminated, caller must
 * free), or NULL on error or timeout.
 */
void *recvuntil(tube *t, const char *delim, double timeout,
                size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2)));

/**
 * @brief Receives one line from the tube.
 *
 * @param t Pointer to the tube.
 * @param timeout Timeout in seconds.
 * @param out_size Optional: Pointer to store the size (can be NULL).
 * @return void* Pointer to the received data (null-terminated, caller must
 * free), or NULL on error.
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
 * @return void** NULL-terminated array of pointers to lines (caller must free
 * each line and the array), or NULL on error.
 */
void **recvlines(tube *t, size_t numlines, double timeout,
                 size_t *out_count) _TUBE_ATTR_NONNULL;

/**
 * @brief Receives all available data from the tube until EOF and closes it.
 *
 * @param t Pointer to the tube.
 * @param timeout Timeout in seconds.
 * @param out_size Optional: Pointer to store the total size (can be NULL).
 * @return void* Pointer to the received data (null-terminated, caller must
 * free), or NULL on error.
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
 * @return void* Pointer to the data received while waiting (null-terminated,
 * caller must free), or NULL on error.
 */
void *sendafter(tube *t, const char *delim, const void *data, size_t size,
                double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * @brief Receives until a delimiter and then sends data followed by a newline.
 *
 * @param t Pointer to the tube.
 * @param delim Delimiter string to wait for.
 * @param data Data to send.
 * @param size Size of the data to send.
 * @param timeout Timeout in seconds for the receive operation.
 * @param out_size Optional: Pointer to store the size of the data received
 * while waiting.
 * @return void* Pointer to the data received while waiting (null-terminated,
 * caller must free), or NULL on error.
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
 * @return void* Pointer to the data received after sending (null-terminated,
 * caller must free), or NULL on error.
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
 * @return void* Pointer to the data received after sending (null-terminated,
 * caller must free), or NULL on error.
 */
void *sendlinethen(tube *t, const char *delim, const void *data, size_t size,
                   double timeout, size_t *out_size) _TUBE_ATTR_NONNULL
    __attribute__((nonnull(2, 3)));

/**
 * @brief Switches the tube to interactive mode, connecting stdin and stdout.
 *
 * @param t Pointer to the tube.
 * @param prompt Prompt string to display (if NULL, default red "$" is used).
 */
void interactive(tube *t, const char *prompt) _TUBE_ATTR_NONNULL;

/**
 * @brief Frees a buffer allocated by a tube function.
 */
void t_free(void *ptr);

/**
 * @brief Frees a NULL-terminated array of lines allocated by t_recvlines.
 */
void t_freelines(void **lines);

/**
 * @brief Closes the tube and cleans up resources.
 */
void t_close(tube *t);

#endif // AXIUM_TUBE_H
