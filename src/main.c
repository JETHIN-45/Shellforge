#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
    char *args[64];
    int count;
} Command;

void parse_command(char *line, Command *cmd) {
    cmd->count = 0;
    char *token = strtok(line, " \t");

    while (token != NULL && cmd->count < 63) {
        cmd->args[cmd->count++] = token;
        token = strtok(NULL, " \t");
    }

    cmd->args[cmd->count] = NULL;
}

int main(void) {
    char *line = NULL;
    size_t len = 0;
    Command cmd;

    while (1) {
        printf("shellforge$ ");
        fflush(stdout);

        if (getline(&line, &len, stdin) == -1)
            break;

        line[strcspn(line, "\n")] = '\0';

        char *args[64];
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

        printf("Command detected: %s | Arguments found: %d\n",
               args[0], i - 1);

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("Command execution error");
            _exit(127);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        } else {
            perror("Fork creation error");
        }

        char *log_line = strdup(line);
        if (log_line == NULL) {
            perror("strdup");
            continue;
        }

        parse_command(log_line, &cmd);

        if (cmd.count > 0) {
            printf("Structure Log -> command: %s | Arguments found: %d\n",
                   cmd.args[0], cmd.count - 1);
        }

        free(log_line);
    }

    free(line);
    return 0;
}
