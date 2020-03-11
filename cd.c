#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


void cd(char* arg){
	// cd(): Changes directory of current process
	pid_t pid = fork();

	if (pid < 0) {
		// Fork failed
		perror("Inside cd(): Fork failed\n");
	} else if (pid > 0) {
		// Parent Process: Wait for child to finish
		wait(NULL);
	} else {
		// Child Process: Change directory of current process
		int suc1 = chdir(arg);
		char path[100];
		char* suc2 = getcwd(path,100);

		if (suc1 != 0) {
			perror("Inside cd(): Couldn't change directory\n");
		}
		if (suc2 == NULL) {
			perror("Inside cd(): Couldn't find current directory\n");
		} else {
			// Print new directory
			printf("%s\n",path);
		}
		// Terminate child process
		exit(0);
	}
}


int main(int argc, char** argv){
	if(argc<2){
		printf("Pass the path as an argument\n");
		return 0;
	}
	cd(argv[1]);
	return 0;
}
