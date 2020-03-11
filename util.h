#ifndef _UTIL_H
#define _UTIL_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#define MAX_CMD_LEN 256
#define MAX_TOKENS 100
#define REDIR_C 3


// data structure for commands with redirects
typedef struct command {
  char **input;
  char **output;
  char **rest;
  int input_len;
  int output_len;
  int rest_len;
} cmd_t;


enum command_type {EXIT=1, CD=2, WC=3, LS=4, OTHER=5};

// |, >, >>
extern char *redir[];

// global vars to save absolute paths of ls and wc,
// needed to exec ls and wc from anywhere
extern char LS_PATH[PATH_MAX], WC_PATH[PATH_MAX];

/*
 * Parses the input string passed to it
 * Takes in a pointer to the array of tokens
 * returns the total number of tokens read
 */
int parse_line(char * input, char * tokens[], char * delim);


/* Returns the command type of the command passed to it */
enum command_type get_command_type(char *command);


// Removes the leading and trailing white spaces in the give string.
// This function modifies the given string.
char *trimwhitespace(char *str);


// changes working directory of current process.
void cd(char *args[], int argc);


// Splits array at midpoint. On completion, head contains elements [0..midpoint-1] and
// tail contains [midpoint+1...len]. Returns length of tail on success, -1 on failure.
int split(char *arr[], int len, int midpoint, char **head[], char **tail[]);


// conditionally set args based on command type
int cmd_check(char **args[]);


// prints a string array to given stream
void print_string_arr(FILE *output, char *arr[], int len);


// pipe the output of one command to the input of another command
int pipe_commands(cmd_t cmd);


// wrapper over execvp that handles errors
int handle_exec(char *args[], int argc);


// execs given arguments in child process, args[0] corresponds to process name
int execute_command(char *args[], int argc);


// writes the output of command to a file
int write_file(cmd_t cmd, char *filename, char *action);

#endif
