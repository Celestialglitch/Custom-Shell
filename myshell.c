#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_CMDS 10
#define MAX_ARGS 64

void sigint_handler(int sig)
{
    printf("\nUse 'exit' to quit the shell.\nmyshell> ");
    fflush(stdout);
}

void parse_command(char *cmd, char **args)
{
    int i = 0;
    args[i] = strtok(cmd, " \t\r\n");
    while (args[i] != NULL)
        args[++i] = strtok(NULL, " \t\r\n");
}

int is_background(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

void handle_redirection(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], "<") == 0)
        {
            int fd = open(args[i + 1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

int main()
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
        if (fgets(input, MAX_CMD_LEN, stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0)
            continue;

        int num_cmds = 0;
        cmds[num_cmds] = strtok(input, "|");
        while (cmds[num_cmds] != NULL)
            cmds[++num_cmds] = strtok(NULL, "|");

        for (int i = 0; i < num_cmds; i++)
        {
            parse_command(cmds[i], args[i]);
        }

        if (num_cmds == 1)
        {
            if (args[0][0] == NULL)
                continue;
            if (strcmp(args[0][0], "exit") == 0)
                break;
            if (strcmp(args[0][0], "cd") == 0)
            {
                if (args[0][1] == NULL)
                {
                    fprintf(stderr, "cd: missing argument\n");
                }
                else
                {
                    if (chdir(args[0][1]) != 0)
                        perror("cd");
                }
                continue;
            }
        }

        for (int i = 0; i < num_cmds - 1; i++)
        {
            pipe(pipefd[i]);
        }

        for (int i = 0; i < num_cmds; i++)
        {
            pid = fork();
            if (pid == 0)
            {

                if (i > 0)
                {
                    dup2(pipefd[i - 1][0], STDIN_FILENO);
                }

                if (i < num_cmds - 1)
                {
                    dup2(pipefd[i][1], STDOUT_FILENO);
                }

                for (int j = 0; j < num_cmds - 1; j++)
                {
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }

                handle_redirection(args[i]);

                int bg = is_background(args[i]);

                execvp(args[i][0], args[i]);
                perror("exec");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < num_cmds - 1; i++)
        {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }

        int bg = is_background(args[num_cmds - 1]);
        if (!bg)
        {
            for (int i = 0; i < num_cmds; i++)
            {
                wait(NULL);
            }
        }
    }

    return 0;
}