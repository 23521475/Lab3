#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

pid_t child_pid = -1;

// Signal handler for Ctrl+C (SIGINT)
void handle_sigint(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGKILL);
        child_pid = -1;
    } else {
        printf("\nCaught Ctrl+C. Type 'exit' to quit.\n");
    }
}

// Signal handler for Ctrl+\ (SIGQUIT)
void handle_sigquit(int sig) {
    printf("\nExiting shell.\n");
    exit(0);
}

// Function to execute a single command
void execute_command(char *command) {
    char *args[MAX_ARGS];
    char *token = strtok(command, " ");
    int i = 0;

    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

// Function to handle input/output redirection
void handle_redirection(char *command) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *temp_command = strdup(command);

    char *token = strtok(temp_command, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            input_file = strtok(NULL, " ");
        } else if (strcmp(token, ">") == 0) {
            output_file = strtok(NULL, " ");
        }
        token = strtok(NULL, " ");
    }

    if (input_file) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror("Input file open failed");
            exit(1);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (output_file) {
        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror("Output file open failed");
            exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    free(temp_command);
}

// Function to handle pipes
void handle_pipe(char *command) {
    char *commands[2];
    commands[0] = strtok(command, "|");
    commands[1] = strtok(NULL, "|");

    if (!commands[1]) {
        perror("Invalid pipe command");
        return;
    }

    int pipefd[2];
    pipe(pipefd);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execute_command(commands[0]);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execute_command(commands[1]);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// Function to parse and execute a command
void parse_and_execute(char *command) {
    // Handle backquotes
    if (strchr(command, '`')) {
        char *start = strchr(command, '`') + 1;
        char *end = strchr(start, '`');

        if (!end) {
            fprintf(stderr, "Error: unmatched backquote\n");
            return;
        }

        char sub_command[MAX_COMMAND_LENGTH];
        strncpy(sub_command, start, end - start);
        sub_command[end - start] = '\0';

        FILE *fp = popen(sub_command, "r");
        if (!fp) {
            perror("popen failed");
            return;
        }

        char result[MAX_COMMAND_LENGTH];
        fgets(result, sizeof(result), fp);
        pclose(fp);

        *start = '\0';
        strcat(command, result);
        strcat(command, end + 1);
    }

    // Check for pipes
    if (strchr(command, '|')) {
        handle_pipe(command);
        return;
    }

    // Handle redirection
    char *redirection_command = strdup(command);
    if (strchr(command, '>') || strchr(command, '<')) {
        handle_redirection(command);
        char *base_command = strtok(redirection_command, "<>");
        execute_command(base_command);
        free(redirection_command);
        return;
    }

    // Execute single command
    pid_t pid = fork();
    if (pid == 0) {
        execute_command(command);
    } else {
        child_pid = pid;
        waitpid(pid, NULL, 0);
        child_pid = -1;
    }

    free(redirection_command);
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    // Set up signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, handle_sigquit);

    while (1) {
        printf("it007sh> ");
        fflush(stdout);

        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        // Remove trailing newline
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            break;
        }

        if (strlen(command) > 0) {
            parse_and_execute(command);
        }
    }
    return 0;
}
