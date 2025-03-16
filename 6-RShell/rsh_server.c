#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include "dshlib.h"
#include "rshlib.h"

#define STOP_SERVER_CMD      "stop-server"
#define CD_SUCCESS_MSG       "Directory changed successfully\n"
#define CD_FAIL_MSG          "cd: failed to change directory\n"
#define PIPE_ERROR           "pipe"
#define FORK_ERROR           "fork"
#define DUP2_ERROR           "dup2"
#define EXECVP_ERROR         "execvp"

/* start_server: Boots up the server, processes client requests, then stops the server. */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;
    svr_socket = boot_server(ifaces, port);
    if(svr_socket < 0){
        int err_code = svr_socket;
        return err_code;
    }
    rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);
    return rc;
}

/* stop_server: Closes the server socket. */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/* boot_server: Creates a socket, binds it, and listens for connections. */
int boot_server(char *ifaces, int port){
    int svr_socket;
    struct sockaddr_in server_addr;
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(svr_socket < 0)
        return ERR_RDSH_COMMUNICATION;
    int enable = 1;
    if(setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return ERR_RDSH_COMMUNICATION;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ifaces);
    memset(&(server_addr.sin_zero), 0, 8);
    if(bind(svr_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        return ERR_RDSH_COMMUNICATION;
    if(listen(svr_socket, 5) < 0)
        return ERR_RDSH_COMMUNICATION;
    return svr_socket;
}

/* process_cli_requests: Accepts client connections and processes their requests. */
int process_cli_requests(int svr_socket){
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket;
    int rc;
    while(1) {
         client_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &addr_len);
         if(client_socket < 0)
             return ERR_RDSH_COMMUNICATION;
         rc = exec_client_requests(client_socket);
         if(rc == OK_EXIT)
             break;
    }
    return OK_EXIT;
}

/* exec_client_requests: Processes commands received from the client. */
int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if(io_buff == NULL)
        return ERR_RDSH_COMMUNICATION;
    int recv_size;
    while(1) {
         recv_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
         if(recv_size <= 0)
             break;
         if(strcmp(io_buff, EXIT_CMD) == 0)
             break;
         if(strcmp(io_buff, STOP_SERVER_CMD) == 0) {
             free(io_buff);
             return OK_EXIT;
         }
         if(strncmp(io_buff, "cd", 2) == 0) {
             char *dir = io_buff + 2;
             while(*dir == ' ')
                 dir++;
             if(chdir(dir) == 0)
                 send_message_string(cli_socket, CD_SUCCESS_MSG);
             else
                 send_message_string(cli_socket, CD_FAIL_MSG);
             continue;
         }
         int sent = send(cli_socket, io_buff, recv_size, 0);
         if(!(sent == recv_size)) {
             free(io_buff);
             return ERR_RDSH_COMMUNICATION;
         }
         if(!(send_message_eof(cli_socket) == OK)) {
             free(io_buff);
             return ERR_RDSH_COMMUNICATION;
         }
    }
    free(io_buff);
    return OK;
}

/* send_message_eof: Sends the EOF character to signal end of response. */
int send_message_eof(int cli_socket){
    int bytes_sent = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    if(!(bytes_sent == 1))
        return ERR_RDSH_COMMUNICATION;
    return OK;
}

/* send_message_string: Sends a message followed by an EOF marker. */
int send_message_string(int cli_socket, char *buff){
    int total = strlen(buff);
    int sent = send(cli_socket, buff, total, 0);
    if(!(sent == total))
        return ERR_RDSH_COMMUNICATION;
    return send_message_eof(cli_socket);
}

/* rsh_execute_pipeline: Executes a command pipeline with proper redirection. */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int i;
    int status;
    int num_cmd = clist->num;
    int prev_fd = -1;
    int pipe_fd[2];
    pid_t pid;
    for(i = 0; i < num_cmd; i++) {
        if(i < num_cmd - 1) {
            if(pipe(pipe_fd) < 0) {
                fprintf(stderr, "Error: %s: %s\n", PIPE_ERROR, strerror(errno));
                return ERR_EXEC_CMD;
            }
        }
        pid = fork();
        if(pid < 0) {
            fprintf(stderr, "Error: %s: %s\n", FORK_ERROR, strerror(errno));
            return ERR_EXEC_CMD;
        } else if(pid == 0) {
            if(i == 0) {
                if(dup2(cli_sock, STDIN_FILENO) < 0) {
                    fprintf(stderr, "Error: %s: %s\n", DUP2_ERROR, strerror(errno));
                    exit(ERR_EXEC_CMD);
                }
            } else {
                if(dup2(prev_fd, STDIN_FILENO) < 0) {
                    fprintf(stderr, "Error: %s: %s\n", DUP2_ERROR, strerror(errno));
                    exit(ERR_EXEC_CMD);
                }
            }
            if(i == num_cmd - 1) {
                if(dup2(cli_sock, STDOUT_FILENO) < 0 || dup2(cli_sock, STDERR_FILENO) < 0) {
                    fprintf(stderr, "Error: %s: %s\n", DUP2_ERROR, strerror(errno));
                    exit(ERR_EXEC_CMD);
                }
            } else {
                if(dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
                    fprintf(stderr, "Error: %s: %s\n", DUP2_ERROR, strerror(errno));
                    exit(ERR_EXEC_CMD);
                }
            }
            if(i > 0)
                close(prev_fd);
            if(i < num_cmd - 1) {
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            fprintf(stderr, "execvp: error: %s\n", strerror(errno));
            exit(ERR_EXEC_CMD);
        } else {
            if(i > 0)
                close(prev_fd);
            if(i < num_cmd - 1) {
                close(pipe_fd[1]);
                prev_fd = pipe_fd[0];
            }
        }
    }
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

Built_In_Cmds rsh_match_command(const char *input)
{
    return BI_NOT_BI;
}

Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    return BI_NOT_BI;
}
