#include "util.h"

// save absolute path of the directory shell is called in
// –– allows custom ls and wc to run from anywhere
char LS_PATH[PATH_MAX], WC_PATH[PATH_MAX];


int chain_io(char *args[], int argc, int redir_idxs[], int redir_c) {
/*
    Handles redirection and piping.
    Takes in user arguments and splits them
    around the piping or redirection operator.
*/

  int status = 0;
  cmd_t cmd;
  cmd.input_len = redir_idxs[0];
  char *action = args[cmd.input_len]; // > >> or |

/*
    Split args into two subarrays, [0...redirect]
    and [redirect+1...length]. The first array
    contains the left half including the operator,
    and the second array contains everything on the
    right side of operator.
*/
  cmd.output_len = split(args, argc, cmd.input_len, &cmd.input, &cmd.output);

  // on split error
  if (cmd.output_len < 0) {
    print_string_arr(stderr, args, argc);
    fprintf(stderr, ": command error\n");
    return status;
  }

  // overwrite redirect string for exec
  cmd.input[cmd.input_len] = NULL;

  if (redir_c == 1) {
    /* pipe or file write */

    int cmd1 = cmd_check(&cmd.input);
    int cmd2 = cmd_check(&cmd.output);

    // if one of the commands == EXIT,
    // ignore redirection and return EXIT
    if (cmd1 == EXIT || cmd2 == EXIT) {
      free(cmd.input);
      free(cmd.output);
      return status = EXIT;
    }

    cmd.rest = NULL; // no remaning args with one I/O redirect 

    // case: |
    if (strcmp(action, redir[0]) == 0) {
       int success = pipe_commands(cmd);

       // on fork or pipe fail
       if (success < 0) {
        print_string_arr(stderr, args, argc);
        fprintf(stderr, ": command error\n");
       }
    }
    else {
    /* write output to file */
    char *filename = cmd.output[0];
    status = write_file(cmd, filename, action);
    }
  }   
  else {
    /* pipe and file write */

    char **freeit = cmd.output;
    int rmd_elems = cmd.input_len + 1;

    cmd.output_len = redir_idxs[1];
    char *action = args[cmd.output_len]; // > or >>
    cmd.output_len -= rmd_elems; // idx of second I/O redirect in split array

    // cmd.input = [0...redir1], cmd.output = [redir+1...redir2], rest = [redir2+1...argc]
    cmd.rest_len = split(cmd.output, argc - rmd_elems, cmd.output_len, &cmd.output, &cmd.rest);

    free(freeit); // deallocate pre-split arguments

    if (cmd.rest_len < 0) {
      print_string_arr(stderr, args, argc);
      fprintf(stderr, ": command error\n");
      free(cmd.input);
      return status;
    }
    cmd.output[cmd.output_len] = NULL;
    
    char *filename = cmd.rest[0];
    status = write_file(cmd, filename, action);
    free(cmd.rest);
  }

  free(cmd.input);
  free(cmd.output);
  return status;
}


int parse_input(char *input) {
  /*
      Splits user input into separate arguments and evalutes for
      redirection characters. If I/O redirection is present,
      calls chain_io, else executes command.
      Returns 1 if user wishes to exit terminal. Otherwise, returns 0.
  */
  int status = 0;

  char **args = malloc(MAX_TOKENS * sizeof(char*));

  if (!args) {
    perror("error: allocation failed:\n");
    return status;
  }

  int argc = parse_line(input, args, " \n");
  // empty input
  if (!argc) {
    free(args);
    return status;
  }

  char **resized_args;
  if ((resized_args = realloc(args, (argc + 1) * sizeof(char*))) == NULL) {
    perror("error: allocation failed\n");
    free(args);
    return status;
  }

  args = resized_args;
  args[argc] = NULL;

  // check if input contains |, >, or >>
  int k = 0;
  int redir_idxs[argc], redir_c = 0;
  for (int i = 0; i < argc; i++) {
    for(int j = 0; j < REDIR_C; j++) {
      if(strncmp(args[i], redir[j], REDIR_C) == 0) {
        redir_idxs[k] = i;
        redir_c++;
        k++;
        break;
      }
    }
  }

  // cmd has I/O redirects
  if (redir_c > 0) {
    // fist argument cannot be redir
    if (redir_idxs[0] == 0) {
      fprintf(stderr, "command error: unable to parse\n");
      return status;
    }
    status = chain_io(args, argc, redir_idxs, redir_c);
  }
  else { // no I/O redirects
    int cmd_type = cmd_check(&args);
    switch(cmd_type) {
      case EXIT:
        status = EXIT;
        break;
      case CD:
        cd(args, argc);
        break;
      default:
        execute_command(args, argc);
        break;
    }
  }

  free(args);
  return status;
}


int main() {
  int status = 0;
  char cwd[PATH_MAX], input[MAX_CMD_LEN];

  if (!getcwd(cwd, PATH_MAX)) {
    perror("unable to get working directory:\n");
    return -1;
  }

  int path_len = strlen(cwd);
  strncpy(LS_PATH, cwd, path_len);
  strncpy(WC_PATH, cwd, path_len);
  strncat(LS_PATH, "/ls", 3);
  strncat(WC_PATH, "/wc", 3);

  while (status != EXIT) {
    if (!getcwd(cwd, PATH_MAX)) {
      perror("unable to get working directory:\n");
      return -1;
    }
    // Display current working directory
    printf("[4061-shell]%s $ ", cwd);
    // Read user input from stdin
    fgets(input, MAX_CMD_LEN, stdin);
    // Process input
    status = parse_input(input);
  }

  return 0;
}
