#include <axium/log.h>
#include <axium/timeout.h>
#include <axium/tubes/process.h>
#include <axium/tubes/tube.h>
#include <axium/utils/fiddling.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define _HOT __attribute__((hot))
#define _INLINE static inline __attribute__((always_inline))
#define _FLATTEN __attribute__((flatten))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

_INLINE void _log_debug_data(const char *msg, const void *data, size_t size) {
  if (unlikely(get_log_level() == DEBUG)) {
    log_debug("%s 0x%zx bytes:", msg, size);
    hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
    opts.prefix = "    ";
    hexdump(data, size, &opts);
  }
}

/**
 * @brief Resolves the effective timeout value, handling the TIMEOUT_DEFAULT
 * sentinel.
 */
_INLINE double _get_timeout(tube *t, double timeout) {
  if (timeout == TIMEOUT_DEFAULT) {
    return t->timeout;
  }
  return timeout;
}

/**
 * @brief Waits for data to be available on a file descriptor using poll.
 * @return 1 if data is available, 0 on timeout, -1 on error.
 */
_HOT _INLINE int _wait_read(int fd, double timeout) {
  if (timeout == 0.0)
    return 0; // Immediate check
  struct pollfd pfd = {.fd = fd, .events = POLLIN};
  int timeout_ms;
  if (timeout == TIMEOUT_FOREVER || timeout < 0) {
    timeout_ms = -1;
  } else {
    timeout_ms = (int)(timeout * 1000);
  }
  int ret;
  do {
    ret = poll(&pfd, 1, timeout_ms);
  } while (ret == -1 && errno == EINTR);
  return ret;
}

static _HOT ssize_t _send_raw(tube *t, const void *data, size_t size) {
  if (unlikely(t->write_fd == -1))
    return -1;
  ssize_t total_sent = 0;
  const char *p = (const char *)data;
  while (size > 0) {
    ssize_t sent = write(t->write_fd, p, size);
    if (unlikely(sent <= 0)) {
      if (errno == EINTR)
        continue;
      return -1;
    }
    p += sent;
    size -= (size_t)sent;
    total_sent += sent;
  }
  return total_sent;
}

_HOT ssize_t send(tube *t, const void *data, size_t size) {
  _log_debug_data("Sent", data, size);
  return _send_raw(t, data, size);
}

ssize_t sendline(tube *t, const void *data, size_t size) {
  char stack_buf[1024];
  char *buf;
  if (likely(size < sizeof(stack_buf))) {
    buf = stack_buf;
  } else {
    buf = malloc(size + 1);
    if (unlikely(!buf))
      return -1;
  }
  memcpy(buf, data, size);
  buf[size] = '\n';
  ssize_t n = send(t, buf, size + 1);
  if (unlikely(buf != stack_buf))
    free(buf);
  return n;
}

_HOT ssize_t recv(tube *t, void *buf, size_t size, double timeout) {
  if (unlikely(t->read_fd == -1))
    return -1;
  timeout = _get_timeout(t, timeout);
  if (_wait_read(t->read_fd, timeout) <= 0) {
    return 0; // Timeout or error
  }
  ssize_t n;
  do {
    n = read(t->read_fd, buf, size);
  } while (n == -1 && errno == EINTR);
  if (likely(n > 0)) {
    _log_debug_data("Received", buf, (size_t)n);
  }
  return n;
}

_HOT void *recvuntil(tube *t, const char *delim, double timeout,
                     size_t *out_size) {
  if (unlikely(t->read_fd == -1))
    return NULL;

  timeout = _get_timeout(t, timeout);
  double end_time = 0;
  if (timeout != TIMEOUT_FOREVER && timeout >= 0) {
    end_time = timeout_now() + timeout;
  }

  size_t delim_size = strlen(delim);
  char last_delim_char = delim[delim_size - 1];
  size_t capacity = 1024;
  size_t length = 0;
  unsigned char *buf = malloc(capacity);
  if (unlikely(!buf))
    return NULL;

  while (1) {
    double remaining = TIMEOUT_FOREVER;
    if (end_time != 0) {
      remaining = end_time - timeout_now();
      if (remaining <= 0)
        break;
    }

    if (_wait_read(t->read_fd, remaining) <= 0) {
      // If we didn't read anything yet and it timed out, return NULL.
      // Otherwise, we return the partial data read so far (standard pwntools
      // behavior).
      if (length == 0) {
        free(buf);
        return NULL;
      }
      break;
    }

    unsigned char c;
    ssize_t n = read(t->read_fd, &c, 1);
    if (unlikely(n <= 0)) {
      if (n == -1 && errno == EINTR)
        continue;
      break;
    }

    if (unlikely(length + 1 >= capacity)) {
      capacity *= 2;
      unsigned char *new_buf = realloc(buf, capacity);
      if (unlikely(!new_buf)) {
        free(buf);
        return NULL;
      }
      buf = new_buf;
    }
    buf[length++] = c;

    // Fast path: check last character first
    if (length >= delim_size && c == last_delim_char) {
      if (memcmp(buf + length - delim_size, delim, delim_size) == 0) {
        break;
      }
    }
  }

  if (length > 0) {
    _log_debug_data("Received until delimiter", buf, length);
  }

  if (out_size)
    *out_size = length;

  unsigned char *final_buf = realloc(buf, length + 1);
  if (likely(final_buf)) {
    final_buf[length] = '\0';
    return final_buf;
  }
  buf[length] = '\0';
  return buf;
}

void *recvline(tube *t, double timeout, size_t *out_size) {
  return recvuntil(t, "\n", timeout, out_size);
}

_HOT void **recvlines(tube *t, size_t numlines, double timeout,
                      size_t *out_count) {
  if (unlikely(numlines == 0))
    return NULL;

  timeout = _get_timeout(t, timeout);
  double end_time = 0;
  if (timeout != TIMEOUT_FOREVER && timeout >= 0) {
    end_time = timeout_now() + timeout;
  }

  void **lines = malloc(sizeof(void *) * (numlines + 1));
  if (unlikely(!lines))
    return NULL;

  size_t count = 0;
  while (count < numlines) {
    double remaining = TIMEOUT_FOREVER;
    if (end_time != 0) {
      remaining = end_time - timeout_now();
      if (remaining <= 0)
        break;
    }

    size_t sz;
    void *line = recvline(t, remaining, &sz);
    if (unlikely(!line))
      break;
    lines[count++] = line;
  }
  lines[count] = NULL;

  if (out_count)
    *out_count = count;

  if (unlikely(count == 0)) {
    free(lines);
    return NULL;
  }
  return lines;
}

_HOT void *recvall(tube *t, double timeout, size_t *out_size) {
  if (unlikely(t->read_fd == -1))
    return NULL;
  timeout = _get_timeout(t, timeout);
  double end_time = 0;
  if (timeout != TIMEOUT_FOREVER && timeout >= 0) {
    end_time = timeout_now() + timeout;
  }

  size_t capacity = 4096;
  size_t total = 0;
  unsigned char *buf = malloc(capacity);
  if (unlikely(!buf))
    return NULL;

  while (1) {
    double remaining = TIMEOUT_FOREVER;
    if (end_time != 0) {
      remaining = end_time - timeout_now();
      if (remaining <= 0)
        break;
    }

    if (_wait_read(t->read_fd, remaining) <= 0)
      break;

    if (unlikely(total + 1024 > capacity)) {
      capacity *= 2;
      unsigned char *new_buf = realloc(buf, capacity);
      if (unlikely(!new_buf)) {
        free(buf);
        return NULL;
      }
      buf = new_buf;
    }
    ssize_t n = read(t->read_fd, buf + total, capacity - total);
    if (unlikely(n <= 0)) {
      if (n == -1 && errno == EINTR)
        continue;
      break;
    }
    total += (size_t)n;
  }
  t_close(t);
  if (out_size)
    *out_size = total;
  unsigned char *final_buf = realloc(buf, total + 1);
  if (likely(final_buf)) {
    final_buf[total] = '\0';
    return final_buf;
  }
  buf[total] = '\0';
  return buf;
}

_FLATTEN void *sendafter(tube *t, const char *delim, const void *data,
                         size_t size, double timeout, size_t *out_size) {
  void *res = recvuntil(t, delim, timeout, out_size);
  send(t, data, size);
  return res;
}

_FLATTEN void *sendlineafter(tube *t, const char *delim, const void *data,
                             size_t size, double timeout, size_t *out_size) {
  void *res = recvuntil(t, delim, timeout, out_size);
  sendline(t, data, size);
  return res;
}

_FLATTEN void *sendthen(tube *t, const char *delim, const void *data,
                        size_t size, double timeout, size_t *out_size) {
  send(t, data, size);
  return recvuntil(t, delim, timeout, out_size);
}

_FLATTEN void *sendlinethen(tube *t, const char *delim, const void *data,
                            size_t size, double timeout, size_t *out_size) {
  sendline(t, data, size);
  return recvuntil(t, delim, timeout, out_size);
}

void interactive(tube *t, const char *prompt) {
  log_info("Switching to interactive mode");
  struct pollfd fds[3];
  int nfds = 2;
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = t->read_fd;
  fds[1].events = POLLIN;
  if (t->stderr_fd != -1 && t->stderr_fd != t->read_fd) {
    fds[2].fd = t->stderr_fd;
    fds[2].events = POLLIN;
    nfds = 3;
  }
  unsigned char buf[4096];
  const char *actual_prompt = prompt ? prompt : ANSI_BOLD_RED "λ " ANSI_RESET;
  bool need_prompt = true;
  bool prompt_on_screen = false;

  while (1) {
    if (need_prompt) {
      fputs(actual_prompt, stdout);
      fflush(stdout);
      need_prompt = false;
      prompt_on_screen = true;
    }
    int ret = poll(fds, nfds, -1);
    if (unlikely(ret == -1)) {
      if (errno == EINTR)
        continue;
      break;
    }
    for (int i = 1; i < nfds; i++) {
      if (fds[i].revents & (POLLIN | POLLHUP)) {
        ssize_t n = read(fds[i].fd, buf, sizeof(buf));
        if (likely(n > 0)) {
          if (prompt_on_screen) {
            printf("\r\x1b[K");
            prompt_on_screen = false;
          }
          _log_debug_data("Received", buf, (size_t)n);
          write(STDOUT_FILENO, buf, n);
          if (buf[n - 1] == '\n')
            need_prompt = true;
        } else if (n == 0 || (n == -1 && errno != EINTR)) {
          if (i == 1) {
            log_info("Got EOF while reading in interactive");
            goto end;
          }
        }
      }
    }
    if (fds[0].revents & POLLIN) {
      ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
      if (likely(n > 0)) {
        prompt_on_screen = false;
        if (unlikely(send(t, buf, (size_t)n) <= 0)) {
          log_info("Got EOF while sending in interactive");
          goto end;
        }
        if (n == 1 && buf[0] == '\n')
          need_prompt = true;
      } else if (n == 0) {
        log_info("Got EOF while sending in interactive");
        goto end;
      }
    }
  }
end:
  log_info("Interactive mode ended");
}

void t_free(void *ptr) { free(ptr); }

void t_freelines(void **lines) {
  if (unlikely(!lines))
    return;
  for (int i = 0; lines[i] != NULL; i++) {
    free(lines[i]);
  }
  free(lines);
}

void t_close(tube *t) {
  if (!t)
    return;
  t_cin(t);
  t_cout(t);
  t_cerr(t);
  waitpid(t->pid, NULL, 0);
  free(t);
}
