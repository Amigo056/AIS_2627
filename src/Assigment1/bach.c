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

void waiter(int size, pid_t processes[]) {
    for(int i = 0; i < size; i++) {
        waitpid(processes[i], NULL, 0);
    }
}

void printArgsWithNull(char **args, int size_with_null) {
    if (args == NULL) {
        printf("Array is NULL\n");
        return;
    }

    printf("Arguments: [ ");
    // Percorre até size_with_null para incluir a posição do NULL
    for (int i = 0; i <= size_with_null; i++) {
        if (args[i] == NULL) {
            printf("NULL ");
        } else {
            printf("\"%s\", ", args[i]);
        }
    }
    printf("] | Total elements checked: %d\n", size_with_null + 1);
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

        pid_t processes[size];
        int (*pipefds)[2] = malloc((size - 1) * sizeof(int[2]));

        if (pipefds == NULL) {
            perror("malloc failed");
            continue;
        }

        for (int i = 0; i < size; i++) {
            if (pipe(pipefds[i]) == -1) {
                perror("pipe failed");
                continue;
            }
        }

        if (size == 0) {
            continue;
        }

        if(size > 1){
            for(int i = 0; i < size; i++) {
                char *internal[MAX_ARGS];
                int internalSize = cmdParser(line[i], internal, ">");

                if (internalSize == 1) {
                    char *subInternal[MAX_ARGS];
                    int subInternalSize = cmdParser(internal[0], subInternal, " ");
                    subInternal[subInternalSize] = NULL;

                    printArgsWithNull(subInternal, subInternalSize);

                    processes[i] = fork();
                    if(processes[i] < 0) {
                        perror("fork failed");
                        exit(1);
                    }
                    if (processes[i] == 0) {
                        if(i == 0){
                            dup2(pipefds[i][1], 1);
                            close(pipefds[i][0]);
                        }else if(i == size - 1){
                            dup2(pipefds[i - 1][0], 0);
                            close(pipefds[i - 1][1]);
                        }else {
                            dup2(pipefds[i - 1][0], 0);
                            dup2(pipefds[i][1], 1);
                        }
                        execvp(subInternal[0], subInternal);
                        exit(0);
                    }

                }else {
                    char *subInternal[MAX_ARGS];

                    int subInternalSize = cmdParser(internal[0], subInternal, " ");

                    char *dst = subInternal[subInternalSize - 1];

                    subInternal[subInternalSize] = NULL;

					int fd = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);

                    processes[i] = fork();
                    if(processes[i] < 0) {
                        perror("fork failed");
                        exit(1);
                    }
                    if (processes[i] == 0) {
                        if(i == 0){
                            dup2(pipefds[i][1], 1);
                            close(pipefds[i][0]);
                        }else if(i == size - 1){
                            dup2(pipefds[i - 1][0], 0);
                            close(pipefds[i - 1][1]);
                        }else {
                            dup2(pipefds[i - 1][0], 0);
                            dup2(pipefds[i][1], 1);
                        }
                        execvp(subInternal[0], subInternal);
                        exit(0);
                    }

                }

            }

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
        waiter(size, processes);
	}

    return 0;
}
