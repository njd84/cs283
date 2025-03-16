#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

static int last_rc = 0;

/* --- build_cmd_buff: Parse command line into tokens, handle quotes and redirection --- */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    cmd_buff->argc = 0;
    cmd_buff->input_redir = NULL;
    cmd_buff->output_redir = NULL;
    cmd_buff->output_append = 0;
    while(*cmd_line && isspace((unsigned char)*cmd_line))
        cmd_line++;
    if(*cmd_line=='\0')
        return WARN_NO_CMDS;
    char *end = cmd_line + strlen(cmd_line) - 1;
    while(end > cmd_line && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }
    char *p = cmd_line;
    while(!( *p == '\0'))
    {
        while(!( *p == '\0') && isspace((unsigned char)*p))
            p++;
        if(*p=='\0')
            break;
        char *token = NULL;
        if(*p=='\"')
        {
            p++;
            token = p;
            while(!( *p == '\0') && !(*p == '\"'))
                p++;
            if(*p=='\"')
            {
                *p = '\0';
                p++;
            }
        }
        else
        {
            token = p;
            while(!( *p == '\0') && !isspace((unsigned char)*p))
                p++;
            if(!( *p == '\0'))
            {
                *p = '\0';
                p++;
            }
        }
        if(strcmp(token, "<") == 0)
        {
            while(!( *p == '\0') && isspace((unsigned char)*p))
                p++;
            if(*p=='\0')
                break;
            char *in_token = NULL;
            if(*p=='\"')
            {
                p++;
                in_token = p;
                while(!( *p == '\0') && !(*p == '\"'))
                    p++;
                if(*p=='\"')
                {
                    *p = '\0';
                    p++;
                }
            }
            else
            {
                in_token = p;
                while(!( *p == '\0') && !isspace((unsigned char)*p))
                    p++;
                if(!( *p == '\0'))
                {
                    *p = '\0';
                    p++;
                }
            }
            cmd_buff->input_redir = in_token;
            continue;
        }
        else if((strcmp(token, ">") == 0) || (strcmp(token, ">>") == 0))
        {
            int append = (strcmp(token, ">>") == 0) ? 1 : 0;
            while(!( *p == '\0') && isspace((unsigned char)*p))
                p++;
            if(*p=='\0')
                break;
            char *out_token = NULL;
            if(*p=='\"')
            {
                p++;
                out_token = p;
                while(!( *p == '\0') && !(*p == '\"'))
                    p++;
                if(*p=='\"')
                {
                    *p = '\0';
                    p++;
                }
            }
            else
            {
                out_token = p;
                while(!( *p == '\0') && !isspace((unsigned char)*p))
                    p++;
                if(!( *p == '\0'))
                {
                    *p = '\0';
                    p++;
                }
            }
            cmd_buff->output_redir = out_token;
            cmd_buff->output_append = append;
            continue;
        }
        else
        {
            if(cmd_buff->argc < CMD_ARGV_MAX - 1)
                cmd_buff->argv[cmd_buff->argc++] = token;
            else
                return ERR_CMD_OR_ARGS_TOO_BIG;
        }
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;
    return OK;
}

/* --- build_cmd_list: Split command line by '|' into a command list --- */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    clist->num = 0;
    char *saveptr;
    char *token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while(token != NULL)
    {
        while(*token && isspace((unsigned char)*token))
            token++;
        char *end = token + strlen(token) - 1;
        while(end > token && isspace((unsigned char)*end))
        {
            *end = '\0';
            end--;
        }
        if(strlen(token)==0)
        {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if(!(clist->num == CMD_MAX))
        {
            char *cmd_copy = strdup(token);
            if(cmd_copy == NULL)
                return ERR_MEMORY;
            int ret = build_cmd_buff(cmd_copy, &(clist->commands[clist->num]));
            if(!(ret == OK))
            {
                free(cmd_copy);
                return ret;
            }
            clist->commands[clist->num]._cmd_buffer = cmd_copy;
            clist->num++;
        }
        else
            return ERR_TOO_MANY_COMMANDS;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    if(!(clist->num > 0))
        return WARN_NO_CMDS;
    return OK;
}

/* --- free_cmd_list: Free allocated command list memory --- */
int free_cmd_list(command_list_t *clist)
{
    int i = 0;
    while(i < clist->num)
    {
        free(clist->commands[i]._cmd_buffer);
        clist->commands[i]._cmd_buffer = NULL;
        i++;
    }
    clist->num = 0;
    return OK;
}

/* --- exec_built_in_cmd: Execute built-in commands (rc, cd) --- */
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    if(!(cmd->argc == 0) && strcmp(cmd->argv[0], "rc") == 0)
    {
        printf(RC_PRINT_FMT, last_rc);
        return BI_EXECUTED;
    }
    if(!(cmd->argc == 0) && strcmp(cmd->argv[0], "cd") == 0)
    {
        switch(cmd->argc)
        {
            case 1: return BI_EXECUTED;
            case 2:
                if(!(chdir(cmd->argv[1]) == 0))
                    fprintf(stderr, ERR_CD_DIR, cmd->argv[1]);
                return BI_EXECUTED;
            default:
                fprintf(stderr, ERR_CD_ARGS);
                return BI_EXECUTED;
        }
    }
    return BI_NOT_BI;
}

/* --- execute_pipeline: Fork processes, set up pipes, apply redirection, execute commands --- */
int execute_pipeline(command_list_t *clist)
{
    int num_cmds = clist->num, i;
    pid_t pid;
    pid_t pids[CMD_MAX] = {0};
    int pipefd[2], prev_fd = -1;
    for(i = 0; i < num_cmds; i++)
    {
        if(!(i == (num_cmds - 1)))
        {
            if(pipe(pipefd) < 0)
            {
                return ERR_MEMORY;
            }
        }
        pid = fork();
        if(!(pid >= 0))
        {
            return ERR_MEMORY;
        }
        else if(pid == 0)
        {
            if(!(i == 0))
            {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            if(!(i == (num_cmds - 1)))
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            cmd_buff_t *current = &clist->commands[i];
            if(!(current->input_redir == NULL))
            {
                int fd_in = open(current->input_redir, O_RDONLY);
                if(!(fd_in >= 0))
                {
                    exit(ERR_EXEC_CMD);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            if(!(current->output_redir == NULL))
            {
                int flags = O_WRONLY | O_CREAT;
                flags |= current->output_append ? O_APPEND : O_TRUNC;
                int fd_out = open(current->output_redir, flags, 0644);
                if(!(fd_out >= 0))
                {
                    exit(ERR_EXEC_CMD);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            execvp(current->argv[0], current->argv);
            fprintf(stderr, "execvp: error: %s\n", strerror(errno));
            exit(ERR_EXEC_CMD);
        }
        else
        {
            pids[i] = pid;
            if(!(i == 0))
                close(prev_fd);
            if(!(i == (num_cmds - 1)))
            {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
        }
    }
    for(i = 0; i < num_cmds; i++)
    {
        int wstatus;
        waitpid(pids[i], &wstatus, 0);
        if(i == (num_cmds - 1))
            last_rc = WEXITSTATUS(wstatus);
    }
    return OK;
}

/* --- exec_local_cmd_loop: Main shell loop for reading input and executing commands --- */
int exec_local_cmd_loop()
{
    char cmd_line[SH_CMD_MAX];
    command_list_t clist;
    int first = 1;
    while(1)
    {
        if(isatty(fileno(stdin)) || (!isatty(fileno(stdin)) && !first))
        {
            printf("%s", SH_PROMPT);
            fflush(stdout);
        }
        first = 0;
        if(fgets(cmd_line, sizeof(cmd_line), stdin) == NULL)
        {
            if(!isatty(fileno(stdin)))
            {
                printf("%s", SH_PROMPT);
                fflush(stdout);
            }
            printf("\n");
            break;
        }
        cmd_line[strcspn(cmd_line, "\n")] = '\0';
        if(strlen(cmd_line) == 0)
        {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
        int parse_status = build_cmd_list(cmd_line, &clist);
        if(!(parse_status == OK))
        {
            if(parse_status == WARN_NO_CMDS)
                printf(CMD_WARN_NO_CMD);
            else if(parse_status == ERR_TOO_MANY_COMMANDS)
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            else
                printf(ERR_CMD_PARSE, parse_status);
            continue;
        }
        if(!(clist.num > 0) || strcmp(clist.commands[0].argv[0], EXIT_CMD) == 0)
            break;
        if(exec_built_in_cmd(&clist.commands[0]) == BI_EXECUTED)
        {
            free_cmd_list(&clist);
            continue;
        }
        int exec_status = execute_pipeline(&clist);
        if(!(exec_status == OK))
            printf(CMD_ERR_EXECUTE);
        free_cmd_list(&clist);
    }
    return OK;
}
