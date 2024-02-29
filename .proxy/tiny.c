

#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {
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
}

void doit(int fd){

  int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
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
    read_requesthdrs(&rio);
    
    
    // URI 분석하여 정적/동적인지 구분.
    is_static = parse_uri(uri, filename, cgiargs);
    
    //이 파일이 아예 존재하지 않으면 리턴.
    if(stat(filename, &sbuf)<0){
    	clienterror(fd,filename,"404","Not found",
    		"Tiny couldn't find this file");
   		return;
    }
    
    
    //만약 정적이라면 
    	// 보통 파일인지/읽기권한이 있는지 확인 후 맞다면 서비스함.
    if(is_static){
    	if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
    		clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
    		return;
    	}
    	serve_static(fd, filename, sbuf.st_size, method);
    }
    // 동적이라면 보통 파일인지/실행 가능한지 확인 후 맞다면 서비스함.
    else{
    	if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
    		clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
    		return;
    		}
    	serve_dynamic(fd,filename, cgiargs, method);
}

}
void read_requesthdrs(rio_t *rp){
  char buf[MAXLINE];
    //첫번째 줄은 요청 라인이라 먼저 읽음.
    Rio_readlineb(rp, buf, MAXLINE);

    //요청 헤더를, \r\n이 나올때까지 계속 읽음.
    while(strcmp(buf, "\r\n")){
    	Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs){

  char *ptr;

  //만약 요청이 정적 컨텐츠라면 
  if(!strstr(uri,"cgi-bin")){
    strcpy(cgiargs, "");
      strcpy(filename, ".");
      strcat(filename, uri);
      if(uri[strlen(uri)-1] == '/')
        strcat(filename, "home.html");
      return 1;
  }
  
  //만약 요청이 동적 컨텐츠라면
  else{
    ptr = index(uri, '?');
    if(ptr){
        strcpy(cgiargs, ptr+1);
          *ptr = '\0';
      }
      else{
        strcpy(cgiargs,"");
      }
      strcpy(filename,".");
      strcat(filename, uri);
      return 0;
  }
}


void serve_static(int fd, char *filename, int filesize, char *method){

  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  
  // 파일 타입 결정.
  get_filetype(filename, filetype);
  
  //응답 라인, 헤더 송신.(HTTP 줄이 라인.)
  //(서버에도 같은내용 출력.)
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection : close\r\n", buf);
  sprintf(buf,"%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);
  
  if(strcasecmp(method, "HEAD") == 0)
    return;

  // 요청한 정적 컨텐츠를 찾아서 
  // 가상메모리에 씀 -> 옮겨적음(송신) -> 둘다 닫음.
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  Rio_writen(fd,srcp, filesize);
  Munmap(srcp, filesize);

  // srcp = (char*)Malloc(filesize);
  // Rio_readn(srcfd, srcp, filesize);
  // Close(srcfd);
  // Rio_writen(fd, srcp, filesize);
  // Free(srcp);



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
    else if (strstr(filename, ".mp4"))
      strcpy(filetype, "video/mp4");
    else
    	strcpy(filetype, "text/plain");
}
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method){

  char buf[MAXLINE], *emptylist[] = { NULL };
    
    //응답 라인, 응답 헤더 클라이언트에게 송신.
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    
    //자식 프로세스에서 CGI 프로그램 실행.
    if(Fork() == 0){
      setenv("QUERY_STRING", cgiargs, 1); //환경변수 설정.
      setenv("REQUEST_METHOD", method, 1);
      Dup2(fd, STDOUT_FILENO); // 자식 프로세스의 표준 출력을 클라이언트에게 보낼 fd로 재지정.
      Execve(filename, emptylist, environ); // 자식 프로세스를 새로운 프로그램으로 교체.
        // CGI 파일 경로, 명령 라인 인수 배열, 환경 변수 리스트.
      }
    Wait(NULL);

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