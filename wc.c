#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/*
      Counts lines, words, and characters of file
*/
void count(int *line_count, int *word_count, int *char_count, FILE *stream) {
  *line_count = *word_count = *char_count = 0;
  char *escapes = " \n\r\t\v\f";
  int was_esc = 1;
  char c;

  while ((c = fgetc(stream)) != EOF) {
    if (strchr(escapes, c)) {
       if (c == '\n')
          (*line_count)++;
       was_esc = 1;
    }
    else {
      // a word is a string of one or more non escapes
      *word_count += was_esc;
      was_esc = 0;
    }
    (*char_count)++;
  }
}




/*
      Wc takes a path and corresponding
      flags, if any, and finds word count,
      line count, and/or character count
      of the given file.
*/
void wc(int mode, char* path) {
  FILE *input;
  if (!path) {
    input = stdin;
    path = ""; // prevent printing (null) when reading from stdin
  }
  else {
    input = fopen(path, "r");
    if (!input) {
      fprintf(stderr, "wc: %s: open: No such file or directory\n", path);
      return;
    }
  }

  int line_count, char_count, word_count;
  count(&line_count, &word_count, &char_count, input);

  switch (mode) {
    case 0:
      printf("%8d %7d %7d %s\n", line_count, word_count, char_count, path);
      break;
    case 1:
      printf("%8d %s\n", line_count, path);
      break;
    case 2:
      printf("%8d %s\n", word_count, path);
      break;
    case 3:
      printf("%8d %s\n", char_count, path);
      break;
  }

  if (fclose(input) == EOF) {
    perror("Failed to close file\n");
  }
}



int main(int argc, char** argv){
  // Sets mode based on number of
  // arguments, and calls wc
  if (argc>2) {
	  if (strcmp(argv[1], "-l") == 0) {
		  wc(1, argv[2]);
		} else if(strcmp(argv[1], "-w") == 0) {
		  wc(2, argv[2]);
		} else if(strcmp(argv[1], "-c") == 0) {
		  wc(3, argv[2]);
		} else {
			printf("Invalid arguments\n");
		}
	} else if (argc==2) {
	 	if (strcmp(argv[1], "-l") == 0) {
		  wc(1, NULL);
		} else if(strcmp(argv[1], "-w") == 0) {
		  wc(2, NULL);
		} else if(strcmp(argv[1], "-c") == 0) {
		  wc(3, NULL);
		} else {
    	wc(0, argv[1]);
    	}
  	} else {
  	    wc(0,NULL);
  	}

  return 0;
}
