

#include "csapp.h"

void echo(int connfd){

    size_t n;
	char buf[MAXLINE];
    rio_t rio;
   
   //read 준비
   Rio_readinitb(&rio, connfd);
   
   //데이터가 있는동안 루프 돌리기
   //(받은 데이터 바이트 출력 후 Writen.
   while((n= Rio_readlineb(&rio, buf, MAXLINE)) != 0){
   		printf("server received %d bytes\n", (int)n);
   		Rio_writen(connfd,buf,n);
    }
}

int main(int argc, char **argv)
{
	int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
 
 	//인자 2개만 받을게
    if(argc != 2){
    	fprintf(stderr, "usage: %s <port>\n", argv[0]);
    	exit(0);
    }
    
    // 듣기소켓으로 쓸 주소 가져왔다
    listenfd = Open_listenfd(argv[1]);
    
    //듣기 식별자로 Accept로 연결 식별자 받아와서
    //현재 누구랑 연결되었는지 확인하구
    //연결 식별자를 echo 함수에 넘겨야징
    while(1){
    	clientlen = sizeof(struct sockaddr_storage);
    	connfd = Accept(listenfd, (SA *)&clientaddr, 
        			&clientlen);
    	Getnameinfo((SA *) &clientaddr, clientlen, 
        	client_hostname, MAXLINE,
            client_port, MAXLINE, 0);
    	printf("Connected to (%s, $s)\n", client_hostname, client_port);
    	echo(connfd);
    	Close(connfd);
    }
    exit(0);
    
    }