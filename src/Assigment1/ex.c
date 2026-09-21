#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 256
#define MAX_ARGS 10

typedef struct {
    char *args[MAX_ARGS + 1];
    char *output_file;
} Command;

//Mostra o prompt e le uma linha de comando do utilizador. Retorna 0 se for EOF, 1 caso contrario. */
static int read_line(char *buffer, int capacity) {
    printf("$ ");
    fflush(stdout);

    if (fgets(buffer, capacity, stdin) == NULL) {
        return 0;
    }

    char *newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0';
    } else if (!feof(stdin)) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
        fprintf(stderr, "Linha demasiado longa.\n");
        buffer[0] = '\0';
    }

    return 1;
}

// Separa 
static int split_words(char *text, char *words[], int max_words) {
    int count = 0;
    char *token = strtok(text, " \t\r\n");

    while (token != NULL) {
        if (count == max_words) {
            return -1;
        }
        words[count++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    words[count] = NULL;
    return count;
}

/* Devolve 1 para um comando valido, 0 para linha vazia e -1 para erro. */
static int parse_command(char *line, Command *command) {
    command->output_file = NULL;

    if (strchr(line, '|') != NULL) {
        fprintf(stderr, "Pipes ainda nao implementados.\n");
        return -1;
    }

    char *redirect = strchr(line, '>');
    if (redirect != NULL) {
        *redirect = '\0';
        redirect++;
        char *files[2];

        if (strchr(redirect, '>') != NULL ||
            split_words(redirect, files, 1) != 1) {
            fprintf(stderr, "Usa: comando [argumentos] > ficheiro\n");
            return -1;
        }
        command->output_file = files[0];
    }

    int count = split_words(line, command->args, MAX_ARGS);
    if (count < 0) {
        fprintf(stderr, "Demasiados argumentos (maximo de %d palavras).\n", MAX_ARGS);
        return -1;
    }
    if (count == 0 && command->output_file != NULL) {
        fprintf(stderr, "Falta o comando antes de >.\n");
        return -1;
    }

    return count > 0;
}

static int redirect_output(const char *filename) {
    if (filename == NULL) {
        return 0;
    }

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror(filename);
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
    }
    if (fd != STDOUT_FILENO) {
        close(fd);
    }
    return 0;
}

static void execute_command(const Command *command) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* So o filho altera a sua saida e e substituido pelo programa. */
        if (redirect_output(command->output_file) < 0) {
            _exit(EXIT_FAILURE);
        }
        execvp(command->args[0], command->args);
        perror(command->args[0]);
        _exit(EXIT_FAILURE);
    }

    while (waitpid(pid, NULL, 0) < 0) {
        if (errno != EINTR) {
            perror("waitpid");
            break;
        }
    }
}

int main(void) {
    char line[MAX_LINE];
    Command command;

    while (read_line(line, sizeof(line))) {
        if (parse_command(line, &command) > 0) {
            execute_command(&command);
        }
    }
    return EXIT_SUCCESS;
}
