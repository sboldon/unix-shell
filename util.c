#include "util.h"

// provide standalone cd function for shell use
void cd(char *args[], int argc){
  // cd(): Changes directory of current process
  if (argc > 2)
    fprintf(stderr, "cd: string not in pwd: %s\n", args[1]);
  else if (args[1]) {
    if (chdir(args[1]) < 0)
      perror("cd: couldn't change directory\n");
  }
  else
    fprintf(stderr, "error: no directory path\n");
}


// creates two new arrays, [0...midpoint] and [midpoint+1...len]
int split(char *arr[], int len, int midpoint, char **head[], char **tail[]) {
  *head = malloc((midpoint + 1) * sizeof(char *));
  if (!head) {
    perror("error: allocation failed:\n");
    return -1;
  }

  *tail = malloc((len - midpoint) * sizeof(char*));
  if (!tail) {
    perror("error: allocation failed:\n");
    return -1;
  }

  int tail_len = len - midpoint;
  // copy arr[0...midpoint]
  memcpy(*head, arr, (midpoint + 1) * sizeof(char *));
  // copy arr[midpoint+1...len]
  memcpy(*tail, arr + midpoint + 1, tail_len * sizeof(char *));

  return tail_len;
}


// error handling wrapper around execvp
int handle_exec(char *args[], int argc) {
  execvp(args[0], args);
  // on failure
  print_string_arr(stderr, args, argc);
  fprintf(stderr, ": command not found\n");
  return -1;
}


// stdout input -> stdin output -> stdout
int pipe_commands(cmd_t cmd) {
  char *dflt_err = "unable to pipe commands:\n";
  int fd[2], err = -1;

  if (pipe(fd) < 0) {
    perror(dflt_err);
    return err;
  }

  /* spawn two processes, output of first -> input of second */
  pid_t pid = fork();
  switch (pid) {
    case -1:
      perror("unable to fork new process:\n");
      return err;

    /* child process 1 */
    case 0:
      // close read end
      if (close(fd[0]) < 0) {
        perror(dflt_err);
        return err;
      }

      // save original STDOUT_FILENO incase exec fails
      int std_out;
      if ((std_out = dup(STDOUT_FILENO)) < 0) {
        perror(dflt_err);
        return err;
      }

      // redirect write end of pipe to STDOUT
      if (dup2(fd[1], STDOUT_FILENO) < 0) {
        perror(dflt_err);
        return err;
      }

      // close write end
      if (close(fd[1]) < 0) {
        perror(dflt_err);
        return err;
      }

      if (handle_exec(cmd.input, cmd.input_len) < 0) {
        // normalize STDOUT on exec fail
        dup2(std_out, STDOUT_FILENO);
        close(std_out);
        exit(-1);
      }
      break;
  }

  pid_t pid2 = fork();
  switch (pid2) {
    case -1:
      perror("unable to fork new process:\n");
      return err;

    /* child process 2 */
    case 0:
      if (close(fd[1]) < 0) {
        perror(dflt_err);
        return err;
      }

      int std_out;
      if ((std_out = dup(STDOUT_FILENO)) < 0) {
        perror(dflt_err);
        return err;
      }

      // redirect read end of pipe to STDIN
      if (dup2(fd[0], STDIN_FILENO) < 0) {
        perror(dflt_err);
        return err;
      }

      if (close(fd[0]) < 0) {
        perror(dflt_err);
        return err;
      }

      if (handle_exec(cmd.output, cmd.output_len - 1) < 0) {
        dup2(std_out, STDOUT_FILENO);
        close(std_out);
        exit(-1);
      }
      break;
  }

  /* parent */
  if (close(fd[1]) < 0) {
    perror(dflt_err);
    return err;
  }
  if (close(fd[0]) < 0) {
    perror(dflt_err);
    return err;
  }

  waitpid(pid, NULL, 0);
  waitpid(pid2, NULL, 0);
  return 0;
}


// forks and runs a single process
int execute_command(char *args[], int argc) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("unable to fork new process:\n");
    return -1;
  }
  else if (pid == 0) {
    handle_exec(args, argc);
    exit(0);
  }
  else {
    wait(NULL);
  }
  return 0;
}

// spawns one or two child processes and writes their output to a file
int write_file(cmd_t cmd, char *filename, char *action) {
  int flags;
  // case: >
  if (strcmp(action, redir[1]) == 0) {
    flags = O_CREAT | O_WRONLY | O_TRUNC;
  }
  // case: >>
  else {
    flags = O_CREAT | O_WRONLY | O_APPEND;
  }

  int new_fd = open(filename, flags, S_IRWXU);

  // check for open failure
  if (new_fd < 0) {
    perror("unable to open file:\n");
    return 0;
  }

  // save stdout to be restored
  int std_out = dup(STDOUT_FILENO);
  if (std_out < 0) {
    fprintf(stderr, "failed to write file: %s\n", filename);
    close(new_fd);
    return 0;
  }

  // redirect stdout to new_fd
  if (dup2(new_fd, STDOUT_FILENO) < 0) {
    fprintf(stderr, "failed to write to file: %s\n", filename);
    close(new_fd);
    close(std_out);
    return 0;
  }

  // run process(es)
  int success = 0;
  if (!cmd.rest) {
    success = execute_command(cmd.input, cmd.input_len);
  }
  else {
    success = pipe_commands(cmd);
  }

  // on fork or pipe fail
  if (success < 0) {
    print_string_arr(stderr, cmd.input, cmd.input_len);
    print_string_arr(stderr, cmd.output, cmd.output_len);
    fprintf(stderr, ": command error\n");
  }

  // normalize STDOUT –– failure is terminal
  if (dup2(std_out, STDOUT_FILENO) < 0) {
    perror("failed to restore stdout –– exiting shell:\n");
    close(new_fd);
    close(std_out);
    return EXIT;
  }

  if (close(new_fd) < 0) {
    fprintf(stderr, "failed to close file: %s\n", filename);
    return 0;
  }

  if (close(std_out) < 0) {
    perror("failed to close a file:\n");
    return 0;
  }

  return 0;
}
