#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

/*
	Alternate implementation of ls Unix command.
	Takes path to directory that is to be listed
	recurse_flag = true if user wishes to look at all files recursively
*/

void ls(char *path, bool recurse_flag) {
	/* 
	   path given no recursion
	   iterates through all files in directory and prints their names
	*/
	if (path != NULL && !recurse_flag) {
		struct dirent **namelist;
		int n = scandir (path, &namelist, NULL, alphasort);
		if (n < 0) {
			perror("scan didn't work");
		}
		else {
			for (int i = 0; i < n; i++) {
				if (i >= 2) {
					if ( (((namelist[i]->d_name)[0])- '.') != 0) {
						printf("%s\n", namelist[i]->d_name);
					}
				}
				free(namelist[i]);
			}
			free(namelist);
		}
	}
	
	//path not given no recursion
	if (path == NULL && !recurse_flag) {
		ls("./", false);
	}
	
	/* 
	   path given with recursion
	   prints all files in directory
	   if these files also happen to be directories,
	   contents of subdirectories are also printed
	*/
	if (path != NULL && recurse_flag) {
		printf("%s:\n\n", path);  
	    
		struct dirent **namelist;
		int n = scandir (path, &namelist, NULL, alphasort);
		if (n < 0) {
			perror("scan didn't work");
		}
		else {
			
			//iterates through all files in directory and prints them
			//doesn't recursively call ls again because dirent shouldn't be free'd 
			//doesn't print dotfiles
			for (int i = 2; i < n; i++) {
				if ( (((namelist[i]->d_name)[0])- '.') != 0) {
						printf("%s\n", namelist[i]->d_name);
				}
				
				if (i == (n-1)) {
					printf("%s", "\n");
				}
			}
		
			
			/*
				finds all files that are directories 
				uses nextpath to store the pathname to these directories
				recursively calls ls on all subdirectories using nextpath
			*/
			for (int i = 0; i < n; i++) {
				if (i >= 2) {
					//if file is a dotfile treat it as being hidden
					if ( (((namelist[i]->d_name)[0])- '.') != 0) {
					
						//checks to prevent buffer overflow
						
						if ( strlen(path) + strlen(namelist[i]->d_name) + 1 > PATH_MAX) {
							perror("path is too long\n");
						}
						
					
						char nextpath[PATH_MAX];
						
						//if path is ./ another slash being added would be invalid
						strcpy(nextpath,path);
						if ( strcmp(path,"./") != 0) {
							strcat(nextpath,"/");
						}
						strcat(nextpath, namelist[i]->d_name);
						struct stat stats;
						
						if ( stat(nextpath, &stats) == -1) {
							perror("stat failed possibly dealing with a broken link\n");
						}
						if ( S_ISDIR(stats.st_mode)) {
							ls(nextpath,true);
						}
					
					}
				}
				free(namelist[i]);
			}
			free(namelist);
		}
	}
	
	//pathname not specified recursive flag is set to true
	if (path == NULL && recurse_flag) {
		ls("./" , true);
	}
}

int main(int argc, char *argv[]){

	if(argc < 2){ // No -R flag and no path name
		ls(NULL, false);
	}
   	else if(strcmp(argv[1], "-R") == 0) {
		if(argc == 2) { // only -R flag
			ls(NULL, true);
		} else { // -R flag with some path name
			ls(argv[2], true);
		}
	}
	else { // no -R flag but path name is given
		ls(argv[1], false);
	}
	return 0;
}
