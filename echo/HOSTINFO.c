


/* $begin adder */
#include "csapp.h"

int main(int argc, char **argv)
{
	struct addrinfo *p,*listp, hints;
    char buf[MAXLINE];
    int rc, flags;
    
    // 명령행 인자가 2개가 아니라면 종료한다.
    // 커널에서 인자를 꼭 2개 반환하게 하도록 함.
    if (argc != 2) {
    	fprintf(stderr, "usage: %s <domain name>\n",argv[0]);
        exit(0);
        }
        
    //첫번째 인자의 addrinfo를 얻어온다. 실패시 에러처리.
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0){
        fprintf(stderr, "getadrrinfo error: %s\n", gai_strerror(rc));
    	exit(1);
    }
    
    //만든 리스트 구조체를 순회. 구조체 안 모든 주소를 프린트.
    flags = NI_NUMERICHOST;
    for(p = listp; p; p = p->ai_next){
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
        }
    
    // 메모리 누수 방지용.
    Freeaddrinfo(listp);
    
    exit(0);
    
    }