#include <axium/tubes/process.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((warn_unused_result)) __attribute__((nonnull(1))) tube *
process(char *const argv[], char *const envp[]) {
  int p2c[2] = {-1, -1};
  int c2p[2] = {-1, -1};
  int e2p[2] = {-1, -1};
  int sync_pipe[2] = {-1, -1};
  pid_t pid;
  tube *t;

  t = malloc(sizeof(*t));
  if (t == NULL) {
    perror("malloc");
    return NULL;
  }

  if (pipe2(p2c, O_CLOEXEC) == -1) {
    perror("pipe2 (parent_to_child)");
    goto fail;
  }

  if (pipe2(c2p, O_CLOEXEC) == -1) {
    perror("pipe2 (child_to_parent)");
    goto fail;
  }

  if (pipe2(e2p, O_CLOEXEC) == -1) {
    perror("pipe2 (child_stderr_to_parent)");
    goto fail;
  }

  if (pipe2(sync_pipe, O_CLOEXEC) == -1) {
    perror("pipe2 (sync_pipe)");
    goto fail;
  }

  pid = fork();
  if (pid == -1) {
    perror("fork");
    goto fail;
  }

  if (pid == 0) {
    close(sync_pipe[0]);
    if (dup2(p2c[0], STDIN_FILENO) == -1 || dup2(c2p[1], STDOUT_FILENO) == -1 ||
        dup2(e2p[1], STDERR_FILENO) == -1) {
      int err = errno;
      write(sync_pipe[1], &err, sizeof(err));
      _exit(EXIT_FAILURE);
    }

    if (envp != NULL) {
      execvpe(argv[0], argv, envp);
    } else {
      execvp(argv[0], argv);
    }

    int err = errno;
    write(sync_pipe[1], &err, sizeof(err));
    _exit(EXIT_FAILURE);
  }

  close(sync_pipe[1]);
  sync_pipe[1] = -1;
  int err;
  ssize_t n = read(sync_pipe[0], &err, sizeof(err));
  close(sync_pipe[0]);
  sync_pipe[0] = -1;

  if (n > 0) {
    // Child sent an error
    if (n == sizeof(err)) { // ensure we get the integral errno code
      errno = err;
    }
    perror("process: exec failed");
    waitpid(pid, NULL, 0);
    goto fail;
  } else if (n == -1) {
    perror("read (sync_pipe)");
    waitpid(pid, NULL, 0);
    goto fail;
  }

  close(p2c[0]);
  close(c2p[1]);
  close(e2p[1]);

  t->write_fd = p2c[1];
  t->read_fd = c2p[0];
  t->stderr_fd = e2p[0];
  t->pid = pid;

  return t;

fail:
  if (p2c[0] != -1) {
    close(p2c[0]);
    close(p2c[1]);
  }
  if (c2p[0] != -1) {
    close(c2p[0]);
    close(c2p[1]);
  }
  if (e2p[0] != -1) {
    close(e2p[0]);
    close(e2p[1]);
  }
  if (sync_pipe[0] != -1) {
    close(sync_pipe[0]);
  }
  if (sync_pipe[1] != -1) {
    close(sync_pipe[1]);
  }
  free(t);
  return NULL;
}

__attribute__((nonnull(1))) void t_close(tube *t) {
  t_cin(t);
  t_cout(t);
  t_cerr(t);
  waitpid(t_pid(t), NULL, 0);
  free(t);
}
