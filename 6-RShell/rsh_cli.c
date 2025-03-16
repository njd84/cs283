#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include "dshlib.h"
#include "rshlib.h"

#define NEWLINE "\n"
#define PRINT_RESP_FMT "%.*s"
#define STOP_SERVER_CMD "stop-server"

/* start_client: Creates a TCP socket, connects to the server, and returns the socket descriptor. */
int start_client(char *server_ip, int port){
    int cli_socket;
    struct sockaddr_in server_addr;
    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(cli_socket < 0)
        return ERR_RDSH_CLIENT;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    memset(&(server_addr.sin_zero), 0, 8);
    if(connect(cli_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        return ERR_RDSH_CLIENT;
    return cli_socket;
}

/* exec_remote_cmd_loop: Implements the remote shell command loop. */
int exec_remote_cmd_loop(char *server_ip, int port) {
    char *request_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *resp_buff = malloc(RDSH_COMM_BUFF_SZ);
    if(request_buff == NULL || resp_buff == NULL) {
        free(request_buff);
        free(resp_buff);
        return ERR_MEMORY;
    }
    int cli_socket = start_client(server_ip, port);
    if(cli_socket < 0) {
        free(request_buff);
        free(resp_buff);
        return ERR_RDSH_CLIENT;
    }
    while(1) {
        if(isatty(fileno(stdin))) {
            printf("%s", SH_PROMPT);
            fflush(stdout);
        }
        if(fgets(request_buff, SH_CMD_MAX, stdin) == NULL) {
            if(isatty(fileno(stdin)))
                printf(NEWLINE);
            break;
        }
        request_buff[strcspn(request_buff, "\n")] = '\0';
        int send_len = strlen(request_buff) + 1;
        if(send(cli_socket, request_buff, send_len, 0) != send_len) {
            break;
        }
        int eof_received = 0;
        while(!eof_received) {
            int bytes = recv(cli_socket, resp_buff, RDSH_COMM_BUFF_SZ, 0);
            if(bytes <= 0)
                break;
            if((char)resp_buff[bytes-1] == RDSH_EOF_CHAR) {
                eof_received = 1;
                resp_buff[bytes-1] = '\0';
            }
            printf(PRINT_RESP_FMT, bytes, resp_buff);
        }
        if(strcmp(request_buff, EXIT_CMD) == 0 || strcmp(request_buff, STOP_SERVER_CMD) == 0)
            break;
    }
    close(cli_socket);
    free(request_buff);
    free(resp_buff);
    return OK;
}
