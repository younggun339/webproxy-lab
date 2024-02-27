#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main(int argc, char **argv) {
  printf("%s", user_agent_hdr);

  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
  return 0;
}

void doit(int fd){

  int is_valid;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostname[MAXLINE], path[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;
  
  // 요청 라인을 읽고 분석.
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if(!(strcasecmp(method, "GET") == 0 || strcasecmp(method, "HEAD") == 0)) {
    clienterror(fd, method, "501","Not implemented","Tiny does not implement this method");
    return;
  }

  // 추가 요청 헤더를 서버에 보내기 위해 저장해둔다.
  char *headers[MAXLINE];  // 최대 헤더 개수
  size_t num_headers = 0;

  // 메모리 동적 할당
  for (size_t i = 0; i < MAXLINE; i++) {
      headers[i] = malloc(MAXLINE);
      if (headers[i] == NULL) {
          perror("Memory allocation error");
          return 1;
      }
  }
  read_requesthdrs(&rio, headers, &num_headers);


  // URI 분석하여 도메인/인자를 구분한다.
  is_valid = parse_uri(uri, hostname, path);

  // 근데 여기에 port도 받아와서 경우에 따라 다르게 요청해야겠군...

  //도메인으로 적절한 웹서버와 연결을 맺는다.
	int clientfd;
  char buff[MAXLINE];
  rio_t rio_two;
        
  //fd 받아오고 읽을 준비 중....
  clientfd = Open_clientfd(hostname, 80);
  Rio_readinitb(&rio_two, clientfd);
  
  // 요청 라인, 헤더로 보낼 문장 만들기.
  sprintf(buff, "%s %s HTTP/1.0\r\n", method, path);
  sprintf(buff, "%sHost : %s\r\n", buf, uri);
  sprintf(buff, "%sUser-Agent : %s\r\n", buf, user_agent_hdr);
  sprintf(buff, "%sConnection : close\r\n", buf);
  sprintf(buff, "%sProxy-Connection : close\r\n", buf);
  sprintf(buff, "%s%s\r\n", buf, headers);
  sprintf(buff, "%s\r\n\r\n", buf);
  // 요청 라인, 헤더를 새로운 서버에게 보내주기.
  Rio_writen(clientfd, buff, strlen(buf));

  // 그리고 응답 받을 파일을 만들어서...
  FILE *file = tmfile();
  // 파일 크기 재고...
  fseek(file, 0, SEEK_END); // 파일 끝으로 이동
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // 응답 받은거 file에 저장해가지고..
  Rio_readlineb(clientfd, file, size);

  int srcfd;
  char *srcp;

  //file을 읽은 대로 지금 클라이언트에게 보내줘야지..
  srcfd = Open(file, O_RDONLY, 0);
  srcp = Mmap(0, size, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  Rio_writen(fd,srcp, size);
  Munmap(srcp, size);

  // 다 보냈다 문 닫을게
  Close(clientfd);
  fclose(file);

  // 할당된 메모리 해제
  for (size_t i = 0; i < num_headers; i++) {
      free(headers[i]);
  }
  exit(0);

}
void read_requesthdrs(rio_t *rp,char **headers, size_t *num_headers){
 // 첫번째 줄은 요청 라인이라 먼저 읽음.
    Rio_readlineb(rp, headers[*num_headers], MAXLINE);
    (*num_headers)++;

    // 요청 헤더를 \r\n이 나올때까지 계속 읽음.
    while ((*num_headers) < MAXLINE && strcmp(headers[(*num_headers) - 1], "\r\n") != 0) {
        Rio_readlineb(rp, headers[*num_headers], MAXLINE);
        printf("%s", headers[*num_headers]);
        (*num_headers)++;
    }
}

int parse_uri(char *uri, char *hostname, char *path){

  char *ptr;

  char protocol[MAXLINE];
  if(sscanf(uri, "%[^:]://%[^/]/%s", protocol, hostname, path) == 0){
    return 1;
  }

  return 0;
}

void get_filetype(char *filename, char *filetype){

  	if(strstr(filename, ".html"))
    	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
    	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
    	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
    	strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mpeg"))
      strcpy(filetype, "video/mpeg");
    else
    	strcpy(filetype, "text/plain");
}
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){

char buf[MAXLINE], body[MAXBUF];
    
    //응답 본체 설계.
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff""\r\n", body);
    sprintf(body,"%s$s: $s\r\n", body, errnum, shortmsg);
    sprintf(body,"%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body,"%s<hr><em> The Tiny Web server</em>\r\n", body);
    
    // 응답 출력.
    sprintf(buf,"HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length:%d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body,strlen(body));
}