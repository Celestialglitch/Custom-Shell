
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024 // Maximum length of entire input line
#define MAX_CMDS 10      // Maximum number of piped commands
#define MAX_ARGS 64      // Maximum arguments per command

/*
 * SIGINT handler — prevents Ctrl+C from killing the shell.
 * Instead, reminds the user to use 'exit'.
 */
void sigint_handler(int sig)
{
    (void)sig; // suppress unused warning
    printf("\n[!] Use 'exit' to quit the shell.\nmyshell> ");
    fflush(stdout);
}

/*
 * Splits a command string into tokens (arguments).
 * Example: "ls -l /home" → args = {"ls", "-l", "/home", NULL}
 */
void parse_command(char *cmd, char **args)
{
    int i = 0;
    args[i] = strtok(cmd, " \t\r\n");
    while (args[i] && i < MAX_ARGS - 1)
        args[++i] = strtok(NULL, " \t\r\n");
    args[i] = NULL;
}

/*
 * Checks if command should run in background.
 * Returns 1 if '&' found, and removes it from args.
 */
int is_background(char **args)
{
    for (int i = 0; args[i]; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

/*
 * Handles file redirection: >, >>, and <
 * Redirects stdin/stdout as appropriate.
 */
void handle_redirection(char **args)
{
    for (int i = 0; args[i]; i++)
    {
        if (strcmp(args[i], ">") == 0 && args[i + 1])
        {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
                perror("open");
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], ">>") == 0 && args[i + 1])
        {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0)
                perror("open");
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], "<") == 0 && args[i + 1])
        {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0)
                perror("open");
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

/*
 * Entry point — main shell loop.
 */
int main(void)
{
    char input[MAX_CMD_LEN];
    char *cmds[MAX_CMDS];
    char *args[MAX_CMDS][MAX_ARGS];
    int pipefd[MAX_CMDS - 1][2];
    pid_t pid;

    signal(SIGINT, sigint_handler);

    while (1)
    {
        printf("myshell> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;
        input[strcspn(input, "\n")] = 0; // remove trailing newline
        if (strlen(input) == 0)
            continue;

        // Split input by '|'
        int num_cmds = 0;
        cmds[num_cmds] = strtok(input, "|");
        while (cmds[num_cmds] && num_cmds < MAX_CMDS - 1)
            cmds[++num_cmds] = strtok(NULL, "|");

        // Tokenize each command
        for (int i = 0; i < num_cmds; i++)
            parse_command(cmds[i], args[i]);

        // Handle built-ins (only if single command)
        if (num_cmds == 1)
        {
            if (args[0][0] == NULL)
                continue;

            if (strcmp(args[0][0], "exit") == 0)
                break;

            if (strcmp(args[0][0], "cd") == 0)
            {
                if (!args[0][1])
                    fprintf(stderr, "cd: missing argument\n");
                else if (chdir(args[0][1]) != 0)
                    perror("cd");
                continue;
            }
        }

        // Create pipes if needed
        for (int i = 0; i < num_cmds - 1; i++)
            if (pipe(pipefd[i]) < 0)
                perror("pipe");

        // Spawn processes for each command
        for (int i = 0; i < num_cmds; i++)
        {
            pid = fork();
            if (pid == 0)
            {
                // Child process

                // Connect input from previous pipe
                if (i > 0)
                    dup2(pipefd[i - 1][0], STDIN_FILENO);

                // Connect output to next pipe
                if (i < num_cmds - 1)
                    dup2(pipefd[i][1], STDOUT_FILENO);

                // Close all pipe fds (important)
                for (int j = 0; j < num_cmds - 1; j++)
                {
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }

                // Handle redirection within command
                handle_redirection(args[i]);

                // Background execution check
                is_background(args[i]);

                // Execute command
                execvp(args[i][0], args[i]);
                perror("execvp"); // only runs if exec fails
                exit(EXIT_FAILURE);
            }
        }

        // Parent closes all pipe ends
        for (int i = 0; i < num_cmds - 1; i++)
        {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }

        // Wait for all children (unless last command is background)
        int bg = is_background(args[num_cmds - 1]);
        if (!bg)
        {
            for (int i = 0; i < num_cmds; i++)
                wait(NULL);
        }
    }

    printf("Exiting myshell...\n");
    return 0;
}