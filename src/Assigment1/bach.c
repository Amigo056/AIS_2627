#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdalign.h>

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

void masking(char *str) {
    int i = 0;
    int has_quotes = 0;
	while(str[i] != '\0') {
		if(str[i] == '"' || str[i] == '\'') {
			has_quotes = !has_quotes;
		}
		if(has_quotes) {
			switch (str[i]) {
				case ' ':
					str[i] = '\x01';
					break;
				case '|':
					str[i] = '\x02';
					break;
				case '>':
					str[i] = '\x03';
					break;
			}
		}
		i++;
	}
}


void unmasking(char *str) {
    int i = 0;
	while(str[i] != '\0') {
		switch (str[i]) {
			case '\x01':
				str[i] = ' ';
				break;
			case '\x02':
				str[i] = '|';
				break;
			case '\x03':
				str[i] = '>';
				break;
		}
		i++;
	}

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

char *trimWhitespace(char *str) {
    //remove da esquerda
    while(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;

    if(*str == 0) return str;

    //remove da direita
    char *end = str + strlen(str) - 1;
    while(end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;

    *(end + 1) = 0;
    return str;
}

void trimQuotes(char *str) {
    int len = strlen(str);
    if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"') || (str[0] == '\'' && str[len - 1] == '\''))) {
        for (int i = 0; i < len - 2; i++) {
            str[i] = str[i + 1];
        }
        str[len - 2] = '\0';
    }
}

void printArgsWithNull(char **args, int size_with_null) {
    if (args == NULL) {
        printf("Array is NULL\n");
        return;
    }

    printf("Arguments: [ ");
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
        masking(buffer);
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
                    int subInternalSize = cmdParser(internal[0], subInternal, " \t");
                    subInternal[subInternalSize] = NULL;

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
                        for (int j = 0; j < subInternalSize; j++) {
                            unmasking(subInternal[j]);
                            trimQuotes(subInternal[j]);
                        }
                        execvp(subInternal[0], subInternal);
                        exit(0);
                    }

                }else {
                    char *subInternal[MAX_ARGS];

                    int subInternalSize = cmdParser(internal[0], subInternal, " \t");

                    char *dst = trimWhitespace(subInternal[subInternalSize - 1]);

                    unmasking(dst);
                    trimQuotes(dst);
                    subInternal[subInternalSize] = NULL;
                    for (int j = 0; j < subInternalSize; j++) {
                        unmasking(subInternal[j]);
                        trimQuotes(subInternal[j]);
                    }

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

				for (int i = 0; i < subSubSize; i++) {
					unmasking(subSubLine[i]);
					trimQuotes(subSubLine[i]);
				}
				printArgsWithNull(subSubLine, subSubSize);
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
					char *dst = trimWhitespace(subLine[subSize - 1]);

                    char *subSubLine[MAX_ARGS];

    				int subSubSize = cmdParser(subLine[0], subSubLine ," \t");

					subSubLine[subSubSize] = NULL;
					for (int i = 0; i < subSubSize; i++) {
						unmasking(subSubLine[i]);
						trimQuotes(subSubLine[i]);
					}
					unmasking(dst);
					trimQuotes(dst);
					int fd = open(dst, O_CREAT | O_RDWR | O_TRUNC, 0644);

					if (fd < 0) {
						perror("Erro ao abrir o ficheiro");
						exit(1);
					}
					dup2(fd, 1);
					close(fd);
					execvp(subSubLine[0], subSubLine);
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
