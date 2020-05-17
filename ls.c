#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include "alloc_test.h"

#define CUR_DIR "./"
#define BOLD_BLUE "\033[1;34m"
#define RESET_COLOR "\033[0m"
#define DEFLT_COL_WIDTH 12
#define COL_LIMIT 8


void print_dir(char *path, int colsize) {
  fputs(BOLD_BLUE, stdout);
  printf("%-*s ", colsize, path);
  fputs(RESET_COLOR, stdout);
  fflush(stdout);
}


void dir_cleanup(int start, int num_entries, struct dirent **namelist) {
  for (int i = start; i < num_entries; ++i) {
    free(namelist[i]);
  }
  free(namelist);
}


void subdir_cleanup(int num_subdirs, char** subdirs) {
  for (int i = 0; i < num_subdirs; ++i) {
    if (subdirs[i]) {
      free(subdirs[i]);
    }
  }
  free(subdirs);
}


int filter_dir(const struct dirent *entry) {
  // disregard directories prefixed with '.'
  return entry->d_name[0] - '.';
}


/*
Alternate implementation of ls Unix command.
Takes path to directory that is to be listed
recurse_flag is true if user wishes to look at all files recursively
*/
int ls(char *path, int recurse_flag) {
  // "ls"
  if (!path && !recurse_flag) {
    return ls(CUR_DIR, 0);
  }
  // "ls -R"
  if (!path && recurse_flag) {
    return ls(CUR_DIR, 1);
  }

  if (recurse_flag && strncmp(path, CUR_DIR, 3) != 0 ) {
    printf("%s:\n", path);
  }

  int num_subdirs = 0;
  char **subdirs = NULL;
  int col_idx = 0;
  int col_widths[COL_LIMIT];
  for (int i = 0; i < COL_LIMIT; ++i) {
    col_widths[i] = DEFLT_COL_WIDTH;
  }

  struct dirent **namelist;
  int num_entries = scandir(path, &namelist, filter_dir, alphasort);
  if (num_entries == -1) {
    fprintf(stderr, "Unable to read directory %s contents: %s\n", path, strerror(errno));
  }
  int entry_lens[num_entries];

  // find max width of each column
  for (int i = 0; i < num_entries; ++i) {
    entry_lens[i] = strlen(namelist[i]->d_name);
    if (entry_lens[i] >= col_widths[col_idx]) {
      // extra char needed for space between columns
      col_widths[col_idx] = entry_lens[i] + 1;
    }
    if (++col_idx == COL_LIMIT) {
      col_idx = 0;
    }
  }

  // print entries and determine which are directories
  for (int i = 0, col_idx = 0; i < num_entries; ++i) {
    int next_path_size = strlen(path) + entry_lens[i] + 1;
    int is_cur_dir = (strncmp(path, CUR_DIR, 3) == 0);
    if (!is_cur_dir) {
      next_path_size++;
    }
    char next_path[next_path_size];
    strcpy(next_path, path);
    if (!is_cur_dir) {
      strcat(next_path, "/");
    }
    strcat(next_path, namelist[i]->d_name);
    struct stat info;
    stat(next_path, &info);

    if (S_ISDIR(info.st_mode)) {
      print_dir(namelist[i]->d_name, col_widths[col_idx]);

      if (recurse_flag) {
        char **expand_subdirs = realloc(subdirs, (num_subdirs + 1) * sizeof(char*));
        if (!expand_subdirs) {
          fprintf(stderr, "expand_subdirs realloc failure\n");
          if (subdirs) {
            subdir_cleanup(num_subdirs, subdirs);
          }
          dir_cleanup(i, num_entries, namelist);
          return -1;
        }
        subdirs = expand_subdirs;
        subdirs[num_subdirs] = malloc(next_path_size);
        if (!subdirs[num_subdirs]) {
          fprintf(stderr, "subdirs[%d] allocation failure\n", num_subdirs);
          subdir_cleanup(num_subdirs, subdirs);
          dir_cleanup(i, num_entries, namelist);
          return -1;
        }
        strcpy(subdirs[num_subdirs], next_path);
        ++num_subdirs;
      }
    }
    else {
      // file is not directory
      printf("%-*s ", col_widths[col_idx], namelist[i]->d_name);
    }
    if (++col_idx == COL_LIMIT) {
      col_idx = 0;
      putchar('\n');
    }
    free(namelist[i]);
  }
  free(namelist);
  // space between directory listings
  puts("\n");

  if (recurse_flag) {
    for (int i = 0; i < num_subdirs; ++i) {
      ls(subdirs[i], 1);
      free(subdirs[i]);
    }
    free(subdirs);
  }
  return 0;
}


int main(int argc, char *argv[]){
  // test malloc failure after number of allocations
  // set_fail_after(100);

  int ret_val = 0;
  if(argc < 2){ // No -R flag and no path name
    ret_val = ls(NULL, 0);
  }
  else if(strcmp(argv[1], "-R") == 0) {
    if(argc == 2) { // only -R flag
      ret_val = ls(NULL, 1);
    }
    else { // -R flag with some path name
      ret_val = ls(argv[2], 1);
    }
  }
  else { // no -R flag but path name is given
    ret_val = ls(argv[1], 0);
  }
  return ret_val;
}
