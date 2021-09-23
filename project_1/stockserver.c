/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include "stock.h"

char stock_filepath[] = "./stock.txt";
int global_listenfd;

void sigint_handler(int signum) {
    if (root > 0) {
        backup_BST(root, stock_filepath);
        free_BST(root);
    }
    if (global_listenfd > 0)
        Close(global_listenfd);
    exit(0);
}

int main(int argc, char **argv){ 
    fd_set reads; // 감시할 소켓 목록( 여기서는 소켓의 입력스트림 감시 용도 ) 
    fd_set temps; // reads 변수의 복사본으로 사용도리 변수 
    struct timeval timeout; // 타임 변수 
    int fd_max; // 최대 검사할 파일디스립터의 수 
    int result; // select() 함수의 리턴값 저장 
    
    struct sockaddr_in clientaddr; 
    socklen_t clientlen; 
    int listenfd, connfd; // listening fd, connection(client) fd
    
    Signal(SIGINT, sigint_handler);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /* stock tree */
    root = read_stock_list(stock_filepath);
    
    listenfd = Open_listenfd(argv[1]);

    FD_ZERO(&reads); // reads 를 0 으로 초기화 
    FD_SET(listenfd, &reads); // 서버용소켓을 감시대상으로 등록 
    
    fd_max = listenfd; 
    // 파일디스립터 번호는 2부터 차례대로 등록 -> 마지막에 생성한 소켓의 번호가 총 소켓의 갯수

    while(1){ 
        int fd; 
        char client_hostname[MAXLINE], client_port[MAXLINE];
        
        temps = reads; // fd_set 변수 복사 

        timeout.tv_sec = 5; // 5초 
        timeout.tv_usec = 0; 
        // 검사 갯수는 0 ~ n-1 로 계산되니 때문에 fd_max+1 삽입 
        // 소켓의 입력스트림 검사 
        // 나머지는 검사 안함 
        // 5초 대기 
        
        /* debugging 용도 */
        // char temp_[MAXLINE];
        // sprintf(temp_, "fd_max: %d\n", fd_max);
        // Fputs(temp_, stdout);

        /* select by bitmap (: temps) */
        result = Select(fd_max+1, &temps, 0, 0, &timeout); 
        
        if(result == 0) { 
            //puts("running servser.."); 
            continue; 
        } 
        
        for(fd = 3 ; fd <= fd_max ; fd++){ // 0,1,2: stdin/out/err
            // 검사 갯수만큼 루프문 돌면서 검사 
            // FD_ISSET() 은 리스트에 1로 되어 있는 소켓을 찾는다. 
            // 즉, 클라이언트로부터 데이터가 날라와서 입력스트림에 뭔가 있는가? 
            if(FD_ISSET(fd, &temps) == 0) 
                continue; 
            
            if(fd != listenfd){ 
                // 클라이언트 소켓이면
                if (!stock(fd)) { // 0: EOF
                    Close(fd);
                    FD_CLR(fd, &reads);

                    /* find max file descriptor and set fd_max */
                    if (fd_max == fd) {
                        int temp_max = fd_max;
                        for (int i = temp_max; i >= listenfd; i--) {
                            if (FD_ISSET(i, &reads)) {
                                fd_max = i;
                                break;
                            }
                        }
                    }
                }
            }
            else { 
                // 서버 소켓이면, 연결요청이 들어온것이므로 연결작업시작 
                clientlen = sizeof(struct sockaddr_storage); 
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                            client_port, MAXLINE, 0);
                printf("Connected to (%s, %s), fd: %d\n", client_hostname, client_port, connfd);

                /* 새로 생성한 클라이언트용 소켓을 감시대상에 추가 */
                FD_SET(connfd, &reads); 
                
                /* fd_max (할당된 socket fd 중 최대값으로) 갱신 */
                if(fd_max < connfd) 
                    fd_max = connfd; 
            } 
        }

        if (fd_max == listenfd)
            backup_BST(root, stock_filepath);
    }
}
/* $end echoserverimain */
