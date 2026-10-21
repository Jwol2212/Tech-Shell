/*
 * Name(s): Connor McKennan, Jacob Wolfe, Antonio Hernandez
 * Date: 02/07/2026
 * Description: A basic shell supporting command execution, cd, and I/O redirection.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

struct ShellCommand {
    char *input_file;
    char *output_file;
    char **argv;
    int argc;
};

char *CommandPrompt(void) {
    static char input[1024];
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s$ ", cwd);
    } else {
        printf("$ ");
    }
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return NULL;
    }

    input[strcspn(input, "\n")] = '\0';
    return input;
}

struct ShellCommand ParseCommandLine(char *input) {
    struct ShellCommand cmd = {NULL, NULL, NULL, 0};
    char *token = strtok(input, " \t");

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t");
            if (token == NULL) {
                fprintf(stderr, "Error: missing input filename\n");
                break;
            }
            free(cmd.input_file);
            cmd.input_file = strdup(token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t");
            if (token == NULL) {
                fprintf(stderr, "Error: missing output filename\n");
                break;
            }
            free(cmd.output_file);
            cmd.output_file = strdup(token);
        } else {
            char **new_argv = realloc(cmd.argv, sizeof(char *) * (cmd.argc + 1));
            if (new_argv == NULL) {
                perror("realloc");
                break;
            }
            cmd.argv = new_argv;
            cmd.argv[cmd.argc] = strdup(token);
            if (cmd.argv[cmd.argc] == NULL) {
                perror("strdup");
                break;
            }
            cmd.argc++;
        }
        token = strtok(NULL, " \t");
    }

    if (cmd.argv != NULL) {
        char **new_argv = realloc(cmd.argv, sizeof(char *) * (cmd.argc + 1));
        if (new_argv != NULL) {
            cmd.argv = new_argv;
            cmd.argv[cmd.argc] = NULL;
        } else {
            perror("realloc");
        }
    }

    return cmd;
}

void ExecuteCommand(struct ShellCommand command) {
    if (command.argv == NULL || command.argv[0] == NULL) {
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        if (command.input_file != NULL && freopen(command.input_file, "r", stdin) == NULL) {
            perror(command.input_file);
            exit(EXIT_FAILURE);
        }
        if (command.output_file != NULL && freopen(command.output_file, "w", stdout) == NULL) {
            perror(command.output_file);
            exit(EXIT_FAILURE);
        }

        execvp(command.argv[0], command.argv);
        int err = errno;
        fprintf(stderr, "Error %d (%s)\n", err, strerror(err));
        exit(EXIT_FAILURE);
    }

    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            perror("waitpid");
            break;
        }
    }
}

void FreeCommand(struct ShellCommand *command) {
    for (int i = 0; i < command->argc; i++) {
        free(command->argv[i]);
    }
    free(command->argv);
    free(command->input_file);
    free(command->output_file);
}

int main(void) {
    char *input;

    while (1) {
        input = CommandPrompt();
        if (input == NULL) {
            break;
        }

        struct ShellCommand command = ParseCommandLine(input);

        /* Ignore blank lines safely. */
        if (command.argv == NULL || command.argv[0] == NULL) {
            FreeCommand(&command);
            continue;
        }

        if (strcmp(command.argv[0], "cd") == 0) {
            if (command.argc > 1) {
                if (chdir(command.argv[1]) != 0) {
                    int err = errno;
                    fprintf(stderr, "Error %d (%s)\n", err, strerror(err));
                }
            } else {
                fprintf(stderr, "Error: no directory specified\n");
            }
            FreeCommand(&command);
            continue;
        }

        ExecuteCommand(command);
        FreeCommand(&command);
    }

    return 0;
}
