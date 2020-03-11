#include "util.h"

char *commands[] = {"exit", "cd", "wc", "ls"};
char *redir[] = {"|", ">", ">>"};


int parse_line(char *input, char *tokens[], char *delim) {
    int i = 0;
    char *tok = strtok(input, delim);

    while(tok != NULL) {
        tokens[i] = tok;
        i++;
        tok = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return i;
}


char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// print a string array to stream
void print_string_arr(FILE *output, char *arr[], int len) {
  for (int i = 0; i < len; i++) {
    fprintf(output, "%s ", arr[i]);
  }
}


enum command_type get_command_type(char *command) {
  for (int i=0; i<OTHER; i++) {
    if (strcmp(command, commands[i]) == 0) {
      return i + 1;
    }
  }
  return OTHER;
}


// set path for LS and WC – allows for cleaner main conditonal
int cmd_check(char **args[]) {
  int cmd = get_command_type((*args)[0]);

  switch (cmd)   {
    case EXIT:
      return EXIT;
    case CD:
      return CD;
    case WC:
      (*args)[0] = WC_PATH;
      break;
    case LS:
      (*args)[0] = LS_PATH;
      break;
  }
  return 0;
}
