/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include "stock.h"

#define N 8192

typedef struct thread_info {
    int tid;
    int fd;
} thread_info;

char stock_filepath[] = "./stock.txt";
int check_thread[N];
int thread_cnt;
sem_t mutex_cnt_t;

void sigint_handler(int signum) {
    if (root > 0) {
        backup_BST(root, stock_filepath);
        free_BST(root);
    }
    exit(0);
}

void *thread(void *vargp) {
    Pthread_detach(pthread_self());

    int tid = ((thread_info*)(vargp))->tid;
    int connfd = ((thread_info*)(vargp))->fd;

    free(vargp);

    while (stock(connfd) == 1);

    Close(connfd);

    /* tid-th thread 비우기 */
    check_thread[tid] = 0;

    P(&mutex_cnt_t);
    thread_cnt--;
    V(&mutex_cnt_t);

    return NULL;
}

int main(int argc, char **argv){ 
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; 
    char client_hostname[MAXLINE], client_port[MAXLINE];

    pthread_t thread_arr[N];

    Signal(SIGINT, sigint_handler);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    Sem_init(&mutex_cnt_t, 0, 1);

    root = read_stock_list(stock_filepath);

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        
        P(&mutex_cnt_t);
        if (thread_cnt == 0) {
            backup_BST(root, stock_filepath);
        }
        
        if (thread_cnt == N) {
            V(&mutex_cnt_t);
            continue;
        }
        V(&mutex_cnt_t);

        clientlen = sizeof(struct sockaddr_storage); 
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        /* find blank cell of thread array */
        int i;
        for (i = 0; i < N && check_thread[i]; i++);

        /* show that i-th cell of thread array is used*/
        check_thread[i] = 1;

        thread_info *info = (thread_info*)malloc(sizeof(thread_info));
        
        info->tid = i;
        info->fd = connfd;

        P(&mutex_cnt_t);
        thread_cnt++;
        V(&mutex_cnt_t);

        Pthread_create((thread_arr + i), NULL, thread, info);
    }
    exit(0);
}
/* $end echoserverimain */
