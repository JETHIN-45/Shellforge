#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void) {
    char *line = NULL;
    size_t len = 0;
    char *args[64];

    while (1) {
        printf("shellforge$ ");
        fflush(stdout);

        if (getline(&line, &len, stdin) == -1)
            break;

        line[strcspn(line, "\n")] = '\0';

        int i = 0;
        char *token = strtok(line, " \t");

        while (token != NULL && i < 63) {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }
        args[i] = NULL;

        if (i == 0)
            continue;

        if (strcmp(args[0], "exit") == 0)
            break;

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "shellforge: missing path parameter\n");
            } else if (chdir(args[1]) != 0) {
                perror("Directory change failed");
            }
            continue;
        }

        char *input_file = NULL;
        char *output_file = NULL;
        int append = 0;
        int j = 0;

        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], "<") == 0) {
                if (args[k + 1] == NULL) {
                    fprintf(stderr, "shellforge: missing input file\n");
                    input_file = NULL;
                    break;
                }
                input_file = args[k + 1];
                k++;
            } else if (strcmp(args[k], ">") == 0 ||
                       strcmp(args[k], ">>") == 0) {
                append = strcmp(args[k], ">>") == 0;

                if (args[k + 1] == NULL) {
                    fprintf(stderr, "shellforge: missing output file\n");
                    output_file = NULL;
                    break;
                }

                output_file = args[k + 1];
                k++;
            } else {
                args[j++] = args[k];
            }
        }
        args[j] = NULL;

        if (j == 0)
            continue;

        pid_t pid = fork();

        if (pid == 0) {
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd == -1) {
                    perror("Input redirection failed");
                    _exit(1);
                }
                if (dup2(fd, STDIN_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }

            if (output_file != NULL) {
                int flags = O_WRONLY | O_CREAT |
                            (append ? O_APPEND : O_TRUNC);
                int fd = open(output_file, flags, 0644);

                if (fd == -1) {
                    perror("Output redirection failed");
                    _exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }

            execvp(args[0], args);
            perror("Command execution error");
            _exit(127);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        } else {
            perror("Fork creation error");
        }
    }

    free(line);
    return 0;
}
