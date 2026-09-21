#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int cmdParser(char *input, char *processed[], char *del) {
	char *copy = strdup(input);
	int idx = 0;

	char *token = strtok(copy, del);

	while(token != NULL){
		processed[idx] = strdup(token);
		idx++;
		token = strtok(NULL, del);

		if (processed[idx] == NULL) {
            break;
        }
	}
	return idx;
}

int main(int argc, char *argv[]){
	char buffer[256];

	while (1) {
        printf("$ ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char *line[10];

       int size = cmdParser(buffer, line, " | "); // cat hey.txt  hello.txt -> 1

       if(size > 1){

	   }else {
			char *subLine[10];

			int subSize = cmdParser(line[0], subLine, " > "); // cat hey.txt hello.txt -> 1

			if(subSize == 1){


				char *subSubLine[10];

				int subSubSize = cmdParser(subLine[0], subSubLine ," ");

				int pid = fork();

				if(pid < 0){
					perror("Fork failed");
					return 1;
				}
				else if(pid == 0){
				    subSubLine[subSubSize] = NULL;
					execvp(subSubLine[0], subSubLine);
					exit(0);
				}
				else {
					int status;
					wait(&status);
				}
			}else {
					int pid = fork();

					if(pid < 0){
						perror("Fork failed");
					return 1;
				}else if(pid == 0){
					char *dst = subLine[subSize];
					subLine[subSize] = NULL;
					int fd = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);

    					if (fd < 0) {
    						perror("Erro ao abrir o ficheiro");
    						exit(1);
    					}
    					dup2(fd, 1);
    					close(fd);
    					execvp(subLine[0], subLine);
    					exit(0);

    				}else {
    					int status;
    					wait(&status);
    				}

			}

		}

	}
	 return 0;
}
