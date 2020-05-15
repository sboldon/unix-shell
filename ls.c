#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

void dir_cleanup(int num_files, struct dirent **files) {
  for (int i = 0; i < num_files; ++i) {
    free(files[i]);
  }
  free(files);
}


void subdir_cleanup(int num_subdirs, char** subdirs) {
  for (int i = 0; i < num_subdirs; ++i) {
    free(subdirs[i]);
  }
  free(subdirs);
}

/*
Alternate implementation of ls Unix command.
Takes path to directory that is to be listed
recurse_flag is true if user wishes to look at all files recursively
*/
int ls(char *path, bool recurse_flag) {
  // current directory
  if (!path && !recurse_flag) {
    return ls("./", false);
  }
  // recurse current directory
  if (!path && recurse_flag) {
    return ls("./" , true);
  }

  struct dirent **files;
  int n = scandir(path, &files, NULL, alphasort);
  if (n == -1) {
    fprintf(stderr, "Unable to scan directory: %s: %s\n", path, strerror(errno));
    return -1;
  }

  // iterates through all files in given directory and prints their names
  if (path && !recurse_flag) {
    while (--n) {
      // ignore entries prefixed with '.'
      if (((files[n]->d_name)[0] - '.') != 0) {
        printf("%s\n", files[n]->d_name);
      }
      free(files[n]);
    }
    free(files);
  }

  // recurses through all present subdirectories
  else if (path && recurse_flag) {
    printf("%s:\n", path);

    int num_subdirs = 0;
    char **subdirs = malloc(sizeof(char*));
    if (!subdirs) {
      perror(NULL);
      dir_cleanup(n, files);
      return -1;
    }
    while (--n) {
      if (((files[n]->d_name)[0] - '.') != 0) {
        printf("%s\n", files[n]->d_name);

        int is_dir = 0;
        // check for d_type support
        if (files[n]->d_type == DT_UNKNOWN && files[n]->d_type != DT_LNK) {
          is_dir = (files[n]->d_type == DT_DIR);
        }
        else {
          struct stat info;
          stat(files[n]->d_name, &info);
          is_dir = S_ISDIR(info.st_mode);
        }
        if (is_dir) {
          char next_path[PATH_MAX];
          strcpy(next_path, path);
          // slash needed if path is not current directory
          if (strncmp(path, "./", 2) != 0) {
            strcat(next_path, "/");
          }
          strcat(next_path, files[n]->d_name);

          subdirs[num_subdirs] = malloc(strlen(next_path) + 1);
          if (!subdirs[num_subdirs]) {
            perror(NULL);
            dir_cleanup(n, files);
            subdir_cleanup(num_subdirs, subdirs);
            return -1;
          }
          strcat(subdirs[num_subdirs], next_path);
          ++num_subdirs;
          char **expanded_subdirs = realloc(subdirs, sizeof(subdirs) + sizeof(char*));
          if (!expanded_subdirs) {
            perror(NULL);
            dir_cleanup(n, files);
            subdir_cleanup(num_subdirs, subdirs);
          }
          subdirs = expanded_subdirs;
        }
      }
      free(files[n]);
    }
    free(files);
    printf("\n");

    for (int i = 0; i < num_subdirs; ++i) {
      ls(subdirs[i], true);
      free(subdirs[i]);
    }
    free(subdirs);
  }
  return 0;
}


int main(int argc, char *argv[]){
  int ret_val = 0;
  if(argc < 2){ // No -R flag and no path name
    ret_val = ls(NULL, false);
  }
  else if(strcmp(argv[1], "-R") == 0) {
    if(argc == 2) { // only -R flag
      ret_val = ls(NULL, true);
    } 
    else { // -R flag with some path name
      ret_val = ls(argv[2], true);
    }
  }
  else { // no -R flag but path name is given
    ret_val = ls(argv[1], false);
  }
  return ret_val;
}
