






#include "csapp.h"

int main(int argc, char **argv)
{
	int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;
    
    //인자 3개 주세요
    if(argc != 3){
    	fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    exit(0);
    }
    
    // host가 두번째 인자, port가 세번째 인자.
    host = argv[1];
    port = argv[2];
    
    //fd 받아오고 읽을 준비 중....
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);
    
    //아무것도 안 줄때까지 받은거 써! 그리고 받은거 읽어!
    //그리고 읽는대로 다 콘솔에 써줘!
    while(Fgets(buf, MAXLINE, stdin) != NULL){
    	Rio_writen(clientfd, buf, strlen(buf));
    	Rio_readlineb(&rio, buf, MAXLINE);
    	Fputs(buf, stdout);
    
    }
    // 다 읽었다 돌아갈게
    Close(clientfd);
    exit(0);
    
}