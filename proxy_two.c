#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#if IS_LOCAL_TEST
int is_local_test = 1;
#else
int is_local_test = 0;
#endif

void receive_pass(int clientfd, int serverfd, rio_t *rio_ser);
void Send(int fd);
void read_requesthdrs(rio_t *rp, int fd, char *hostname);
void parse_uri(char *uri, char *hostname, char *port, char *path);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

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
    Send(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
  return 0;
}

void Send(int fd){

  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE];
  char hostname[MAXLINE], path[MAXLINE], port[MAXLINE];
  rio_t rio;
  
  // 요청 라인을 읽고 분석.
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);

  sscanf(buf, "%s %s", method, uri);

  if(!(strcasecmp(method, "GET") == 0)) {
    clienterror(fd, method, "501","Not implemented","Tiny does not implement this method");
    return;
  }

  // URI 분석하여 도메인, 포트, path를 구분한다.
  parse_uri(uri, hostname, port, path);

  //도메인으로 적절한 웹서버와 연결을 맺는다.
  int proxyfd;
  char buf_ser[MAXLINE];
  rio_t rio_pro;
        
  //fd 받아오고....
  proxyfd = Open_clientfd(hostname, port);

  //읽을 준비중...
  Rio_readinitb(&rio_pro, proxyfd);

  // 요청라인 먼저 보내고..
  sprintf(buf_ser, "%s %s HTTP/1.0\r\n", method, path);
  Rio_writen(proxyfd, buf_ser, strlen(buf_ser));

  // 서버에 요청 헤더 보내는중..
  read_requesthdrs(&rio, proxyfd, hostname);

  // 서버 응답 본문을 클라이언트에게 보내는 중..
  receive_pass(fd, proxyfd, &rio_pro);

  Close(proxyfd);
  exit(0);

}

void receive_pass(int clientfd, int serverfd, rio_t *rio_ser){

  char buf[MAXLINE];
  int Content_length_val;
  // 첫번째 줄 응답 라인
  Rio_readlineb(rio_ser, buf, MAXLINE);
  Rio_writen(clientfd, buf, strlen(buf));

  //응답 헤더 보내야지
  while(strcmp(buf, "\r\n") != 0){
    Rio_readlineb(rio_ser, buf, MAXLINE);
    if(strstr(buf, "Content-length:")){
      sscanf(buf, "Content-length: %d", &Content_length_val);
    }
    Rio_writen(clientfd, buf, strlen(buf));
  }

  // 응답 본문 보내야지
  char *bodyp;

  bodyp = (char*)Malloc(Content_length_val);
  Rio_readn(serverfd, bodyp, Content_length_val);
  Rio_writen(clientfd, bodyp, Content_length_val);
  Free(bodyp);

}

void read_requesthdrs(rio_t *rp, int fd, char *hostname){
  char buf[MAXLINE];
  int User_flag, Connect_flag, Procon_flag, Host_flag = 0;
 // 첫번째 줄은 요청 라인이라 먼저 읽음.
    Rio_readlineb(rp, buf, MAXLINE);

    while(strcmp(buf, "\r\n") != 0){
    	Rio_readlineb(rp, buf, MAXLINE);
      
      if(strstr(buf, "Host:")){
        Host_flag = 1;
        sprintf(buf, "Host: %s\r\n", hostname);
        Rio_writen(fd, buf, strlen(buf));
      }
      else if(strstr(buf,"User-Agent:")){
        User_flag = 1;
        sprintf(buf, "User-Agent : %s\r\n", user_agent_hdr);
        Rio_writen(fd, buf, strlen(buf));
      }
      else if(strstr(buf, "Connection:")){
        Connect_flag = 1;
        sprintf(buf, "Connection: close\r\n");
        Rio_writen(fd, buf, strlen(buf));
      }
      else if(strstr(buf, "Proxy-Connection:")){
        Procon_flag = 1;
        sprintf(buf, "Proxy-Connection: close\r\n");
        Rio_writen(fd, buf, strlen(buf));
      }
      else{
        Rio_writen(fd, buf, strlen(buf));
      }
    }
      if(Host_flag == 0){
        sprintf(buf, "Host: %s\r\n", hostname);
        Rio_writen(fd, buf, strlen(buf));
      }
      else if(User_flag == 0){
        sprintf(buf, "User-Agent : %s\r\n", user_agent_hdr);
        Rio_writen(fd, buf, strlen(buf));
      }
      else if(Connect_flag == 0){
        sprintf(buf, "Connection: close\r\n");
        Rio_writen(fd, buf, strlen(buf));
      }
      else if(Procon_flag == 0){
        sprintf(buf, "Proxy-Connection: close\r\n");
        Rio_writen(fd, buf, strlen(buf));
      }
      sprintf(buf, "\r\n");
      Rio_writen(fd, buf, strlen(buf));
    return;
}

void parse_uri(char *uri, char *hostname, char *port, char *path){

//   char port_str[MAXLINE];
// // 포트 파싱
//   if (sscanf(uri, "%*[^:]://%*[^:]:%[^/]/%*s", port_str) == 1) {
//     if (!sscanf(port_str, "%d", port)) {
//         port = 80; // 기본 포트
//     }
//     } 
//   else {
//     port = 80; // 기본 포트
//   }
  

    // host_name의 시작 위치 포인터: '//'가 있으면 //뒤(ptr+2)부터, 없으면 uri 처음부터
  char *hostname_ptr = strstr(uri, "//") ? strstr(uri, "//") + 2 : uri;
  char *port_ptr = strchr(hostname_ptr, ':'); // port 시작 위치 (없으면 NULL)
  char *path_ptr = strchr(hostname_ptr, '/'); // path 시작 위치 (없으면 NULL)
  strcpy(path, path_ptr);

  if (port_ptr) // port 있는 경우
  {
    strncpy(port, port_ptr + 1, path_ptr - port_ptr - 1); 
    strncpy(hostname, hostname_ptr, port_ptr - hostname_ptr);
  }
  else // port 없는 경우
  {
    if (is_local_test)
      strcpy(port, "80"); // port의 기본 값인 80으로 설정
    else
      strcpy(port, "8000");
    strncpy(hostname, hostname_ptr, path_ptr - hostname_ptr);
  }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){

char buf[MAXLINE], body[MAXBUF];
    
    //응답 본체 설계.
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor=\"ffffff\">\r\n", body);
    sprintf(body,"%s%s: $s\r\n", body, errnum, shortmsg);
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