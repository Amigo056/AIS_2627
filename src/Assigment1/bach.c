#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE 256
#define MAX_ARGS 10
#define PATH_MAX 1024
char cwd[PATH_MAX];

int cmdParser(char *input, char *processed[], char *del) {
	char *copy = strdup(input);
	int idx = 0;

	char *token = strtok(copy, del);

	while(token != NULL){
		processed[idx] = strdup(token);
		idx++;
		token = strtok(NULL, del);
	}
	processed[idx] = NULL;
	return idx;
}

void getCurrentPath() {
    char newP[PATH_MAX];
    if (getcwd(newP, sizeof(newP)) == NULL) {
        perror("getcwd failed");
    }
    snprintf(cwd, sizeof(cwd), "\n%s", newP);
}


int main(int argc, char *argv[]){
	char buffer[MAX_LINE];
	getCurrentPath();
	while (1) {
		printf("%s", cwd);
        printf("$ ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char *line[MAX_ARGS];

        int size = cmdParser(buffer, line, "|"); // cat hey.txt  hello.txt -> 1
        if (size == 0) {
            continue;
        }

        if(size > 1){

	    }else {
			char *subLine[MAX_ARGS];

			int subSize = cmdParser(line[0], subLine, ">"); // cat hey.txt hello.txt -> 1
            if (subSize == 0) {
                continue;
            }

			if(subSize == 1){
				char *subSubLine[MAX_ARGS];

				int subSubSize = cmdParser(subLine[0], subSubLine ," \t");
                if (subSubSize == 0) {
                    continue;
                }
                if (strcmp(subSubLine[0], "exit") == 0) {
                    break;
                }

                if(strcmp(subSubLine[0], "cd") == 0) {
                    if(chdir(subSubLine[1]) != 0) {
                        perror("cd failed");
                    }
                    getCurrentPath();
                    continue;
                }

				int pid = fork();

				if(pid < 0){
					perror("Fork failed");
					return 1;
				}
				else if(pid == 0){
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
					}
				else if(pid == 0){
					char *dst = subLine[subSize - 1];

					subLine[subSize - 1] = NULL;
					int fd = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);

					if (fd < 0) {
						perror("Erro ao abrir o ficheiro");
						exit(1);
					}
					dup2(fd, 1);
					close(fd);
					execvp(subLine[0], subLine);
					exit(0);
				}
				else {
					int status;
					wait(&status);
				}

			}

		}
	}
    return 0;
}
