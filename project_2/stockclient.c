/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"
#include "stock.h"

int global_fd;

void sigint_handler(int signum) {
    if (global_fd > 0)
        Close(global_fd);
    exit(0);
}

int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    Signal(SIGINT, sigint_handler);

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];

    global_fd = clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        if (!strcmp(buf, "exit\n")) {
            Close(clientfd);
            exit(0);
        }
        Rio_writen(clientfd, buf, strlen(buf));

        Rio_readlineb(&rio, buf, MAXLINE);
        
        int len = strlen(buf);
        if (buf[len-1] == '\n')
            buf[len-1] = '\0';
        
        int n = stoi(buf);
        for (int i = 0; i < n; i++) {
            Rio_readlineb(&rio, buf, MAXLINE);
            Fputs(buf, stdout);
        }
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
