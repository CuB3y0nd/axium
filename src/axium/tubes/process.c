#include <axium/log.h>
#include <axium/tubes/process.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

__attribute__((warn_unused_result)) tube *
process_ext(char *const argv[], char *const envp[], tube_flags_t flags) {
  int pipes[3][2] = {{-1, -1}, {-1, -1}, {-1, -1}}; // stdin, stdout, stderr
  int sync_pipe[2] = {-1, -1};
  pid_t pid;
  tube *t = malloc(sizeof(*t));
  if (!t) {
    log_error("malloc failed for tube struct");
    return NULL;
  }

  t->write_fd = t->read_fd = t->stderr_fd = -1;

  // Create necessary pipes based on flags
  tube_flags_t flag_list[] = {TUBE_STDIN, TUBE_STDOUT, TUBE_STDERR};
  for (int i = 0; i < 3; i++) {
    if ((flags & flag_list[i]) && pipe2(pipes[i], O_CLOEXEC) == -1) {
      log_exception("Failed to create pipe %d", i);
      goto fail;
    }
  }
  if (pipe2(sync_pipe, O_CLOEXEC) == -1) {
    log_exception("Failed to create sync pipe");
    goto fail;
  }

  pid = fork();
  if (pid == -1) {
    log_exception("fork failed");
    goto fail;
  }

  if (pid == 0) {
    close(sync_pipe[0]);

    if (flags & TUBE_STDIN)
      dup2(pipes[0][0], STDIN_FILENO);
    if (flags & TUBE_STDOUT)
      dup2(pipes[1][1], STDOUT_FILENO);
    if (flags & TUBE_STDERR)
      dup2(pipes[2][1], STDERR_FILENO);

    for (int i = 0; i < 3; i++) {
      if (pipes[i][0] != -1) {
        close(pipes[i][0]);
        close(pipes[i][1]);
      }
    }

    if (envp)
      execvpe(argv[0], argv, envp);
    else
      execvp(argv[0], argv);

    // If exec fails, notify parent
    int err = errno;
    write(sync_pipe[1], &err, sizeof(err));
    _exit(EXIT_FAILURE);
  }

  close(sync_pipe[1]);
  int exec_err;
  ssize_t n = read(sync_pipe[0], &exec_err, sizeof(exec_err));
  close(sync_pipe[0]);

  if (n > 0) { // Child signaled an exec failure
    if (n == sizeof(exec_err)) {
      errno = exec_err;
    }
    log_exception("process: exec failed for '%s'", argv[0]);
    waitpid(pid, NULL, 0);
    goto fail;
  } else if (n == -1) { // Read error
    log_exception("process: failed to read from sync pipe");
    waitpid(pid, NULL, 0);
    goto fail;
  }

  if (flags & TUBE_STDIN) {
    t->write_fd = pipes[0][1];
    close(pipes[0][0]);
  }
  if (flags & TUBE_STDOUT) {
    t->read_fd = pipes[1][0];
    close(pipes[1][1]);
  }
  if (flags & TUBE_STDERR) {
    t->stderr_fd = pipes[2][0];
    close(pipes[2][1]);
  }

  t->pid = pid;
  return t;

fail:
  for (int i = 0; i < 3; i++) {
    if (pipes[i][0] != -1) {
      close(pipes[i][0]);
      close(pipes[i][1]);
    }
  }
  if (sync_pipe[0] != -1)
    close(sync_pipe[0]);
  if (sync_pipe[1] != -1)
    close(sync_pipe[1]);
  free(t);
  return NULL;
}

tube *process(char *const argv[], char *const envp[]) {
  return process_ext(argv, envp, TUBE_ALL);
}
