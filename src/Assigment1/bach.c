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

int cmdProcessor(char *input, char *processed[], char *del) {
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
    processed[idx] = NULL;
    return idx;
}

int main(int argc, char *argv[]){
    char buffer[MAX_LINE];

    while (1) {
        printf("$ ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char *line[MAX_ARGS];

        int size = cmdProcessor(buffer, line, " | ");

        if(size > 1){

        }else {
            char *subLine[MAX_ARGS];

            int subSize = cmdProcessor(line[0], subLine, " > ");

            if(subSize == 1){
                char *subSubLine[MAX_ARGS];

                int subSubSize = cmdProcessor(subLine[0], subSubLine ," ");

                int pid = fork();

                if(pid < 0){

                    perror("Fork failed");
                    return 1;
                }
                else if(pid ==0){
                    execvp(subSubLine[0], subSubLine);
                    exit(0);
                }
                else {
                    int status;
                    pid_t filho_terminado = wait(&status);
                }
            }else {
                char *dst = subLine[subSize];
                subLine[subSize] = NULL;
                int fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd < 0) {
                    perror("Erro ao abrir o ficheiro");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                execvp(subLine[0], subLine);
            }

        }

        return 0;

    }
}















