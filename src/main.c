#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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
    ssize_t nread;
    Command cmd;

    while (1) {
        printf("shellforge$ ");
        fflush(stdout);

        nread = getline(&line, &len, stdin);
        if (nread == -1)
            break;

        if (nread > 0 && line[nread - 1] == '\n')
            line[nread - 1] = '\0';

        parse_command(line, &cmd);

        if (cmd.count == 0)
            continue;

        if (strcmp(cmd.args[0], "exit") == 0)
            break;

        printf("Structure Log -> command: %s | Arguments found: %d\n",
               cmd.args[0], cmd.count - 1);

        for (int j = 0; j < cmd.count; j++) {
            printf(" -> args[%d]: %s\n", j, cmd.args[j]);
        }
    }

    free(line);
    return 0;
}
