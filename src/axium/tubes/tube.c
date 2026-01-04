#include <axium/log.h>
#include <axium/tubes/process.h>
#include <axium/tubes/tube.h>
#include <axium/utils/fiddling.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define _HOT __attribute__((hot))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static inline void _t_log_debug_data(const char *msg, const void *data,
                                     size_t size) {
  if (unlikely(get_log_level() == DEBUG)) {
    log_debug("%s 0x%zx bytes:", msg, size);
    hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
    opts.prefix = "    ";
    hexdump(data, size, &opts);
  }
}

static _HOT ssize_t _t_send_raw(tube *t, const void *data, size_t size) {
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

_HOT ssize_t t_send(tube *t, const void *data, size_t size) {
  _t_log_debug_data("Sent", data, size);
  return _t_send_raw(t, data, size);
}

ssize_t t_sendline(tube *t, const void *data, size_t size) {
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
  ssize_t n = t_send(t, buf, size + 1);
  if (unlikely(buf != stack_buf))
    free(buf);
  return n;
}

_HOT ssize_t t_recv(tube *t, void *buf, size_t size) {
  if (unlikely(t->read_fd == -1))
    return -1;
  ssize_t n;
  do {
    n = read(t->read_fd, buf, size);
  } while (n == -1 && errno == EINTR);
  if (likely(n > 0)) {
    _t_log_debug_data("Received", buf, (size_t)n);
  }
  return n;
}

void *t_recvuntil(tube *t, const char *delim, size_t *out_size) {
  if (unlikely(t->read_fd == -1 || !delim))
    return NULL;

  size_t delim_size = strlen(delim);
  size_t capacity = 1024;
  size_t length = 0;
  unsigned char *buf = malloc(capacity);
  if (unlikely(!buf))
    return NULL;

  while (1) {
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

    if (length >= delim_size) {
      if (memcmp(buf + length - delim_size, delim, delim_size) == 0) {
        break;
      }
    }
  }

  if (length > 0) {
    _t_log_debug_data("Received until delimiter", buf, length);
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

void *t_recvline(tube *t, size_t *out_size) {
  return t_recvuntil(t, "\n", out_size);
}

void **t_recvlines(tube *t, size_t numlines, size_t *out_count) {
  if (unlikely(numlines == 0))
    return NULL;

  void **lines = malloc(sizeof(void *) * (numlines + 1));
  if (unlikely(!lines))
    return NULL;

  size_t count = 0;
  while (count < numlines) {
    size_t sz;
    void *line = t_recvline(t, &sz);
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

void *t_recvall(tube *t, size_t *out_size) {
  if (unlikely(t->read_fd == -1))
    return NULL;
  size_t capacity = 4096;
  size_t total = 0;
  unsigned char *buf = malloc(capacity);
  if (unlikely(!buf))
    return NULL;
  while (1) {
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

void *t_sendafter(tube *t, const char *delim, const void *data, size_t size,
                  size_t *out_size) {
  void *res = t_recvuntil(t, delim, out_size);
  t_send(t, data, size);
  return res;
}

void *t_sendlineafter(tube *t, const char *delim, const void *data, size_t size,
                      size_t *out_size) {
  void *res = t_recvuntil(t, delim, out_size);
  t_sendline(t, data, size);
  return res;
}

void *t_sendthen(tube *t, const char *delim, const void *data, size_t size,
                 size_t *out_size) {
  t_send(t, data, size);
  return t_recvuntil(t, delim, out_size);
}

void *t_sendlinethen(tube *t, const char *delim, const void *data, size_t size,
                     size_t *out_size) {
  t_sendline(t, data, size);
  return t_recvuntil(t, delim, out_size);
}

void t_interactive(tube *t, const char *prompt) {
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
  const char *actual_prompt = prompt ? prompt : ANSI_BOLD_RED "$ " ANSI_RESET;
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
          _t_log_debug_data("Received", buf, (size_t)n);
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
        if (unlikely(t_send(t, buf, (size_t)n) <= 0)) {
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

void t_free_lines(void **lines) {
  if (unlikely(!lines))
    return;
  for (int i = 0; lines[i] != NULL; i++) {
    free(lines[i]);
  }
  free(lines);
}

void t_close(tube *t) {
  t_cin(t);
  t_cout(t);
  t_cerr(t);
  waitpid(t->pid, NULL, 0);
  free(t);
}
