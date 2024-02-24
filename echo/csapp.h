/*
 * csapp.h - prototypes and definitions for the CS:APP3e book
 */
/* $begin csapp.h */
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//기본 파일 권한 명시를 정의.
// DEF_UMASK는 그 중 그룹과 기타 사용자에 대한 쓰기 권한만 제한하기 위해 따로 편의성으로 빼놓음.
/* Default file permissions are DEF_MODE & ~DEF_UMASK */
/* $begin createmasks */
#define DEF_MODE   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK  S_IWGRP|S_IWOTH
/* $end createmasks */


// sockaddr 구조체를 쉽게 명시하기 위한 정의.
/* Simplifies calls to bind(), connect(), and accept() */
/* $begin sockaddrdef */
typedef struct sockaddr SA;
/* $end sockaddrdef */

// 안정성을 위해 따로 설정. 
/* Persistent state for the robust I/O (Rio) package */
/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */// 즉 어떤 fd와 관련이 있는지 명시.
    int rio_cnt;               /* Unread bytes in internal buf */ // 버퍼에 남은 아직 안 읽은 바이트수.
    char *rio_bufptr;          /* Next unread byte in internal buf */ // 버퍼에서 다음에 읽어들일 데이터 시작위치, 바이트의 포인터.
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */ // 데이터를 저장하는 내부버퍼. 
} rio_t;
/* $end rio_t */

//외부 변수. 다른 소스파일에서 정의되었으며, 참조만 가능한 값.
/* External variables */
extern int h_errno;    /* Defined by BIND for DNS errors */ 
extern char **environ; /* Defined by libc */ // 프로세스 환경병수 가리킴. C라이브러리에 저장되어있음.

/* Misc constants */
#define	MAXLINE	 8192  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */
#define LISTENQ  1024  /* Second argument to listen() */

// 에러 핸들러.
/* Our own error-handling functions */
void unix_error(char *msg);
void posix_error(int code, char *msg);
void dns_error(char *msg);
void gai_error(int code, char *msg);
void app_error(char *msg);



/* Process control wrappers */
pid_t Fork(void);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status); // 부모 프로세스가 자식 프로세스 종료시 사용. status는 자식 프로세스 상태를 가리키는 포인터.
pid_t Waitpid(pid_t pid, int *iptr, int options); //특정 PID 자식 프로세스가 종료될때까지 대기. options는 대기 동작을 조절하는 옵션 플래그.
void Kill(pid_t pid, int signum);
unsigned int Sleep(unsigned int secs); // 주어진 초 동안 지연 시킴.
void Pause(void); // 당장 일단 멈춤.
unsigned int Alarm(unsigned int seconds);
void Setpgid(pid_t pid, pid_t pgid);
pid_t Getpgrp();

// 시그널 래퍼 함수. 
/* Signal wrappers */
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigsuspend(const sigset_t *set);

/* Sio (Signal-safe I/O) routines */
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);

/* Sio wrappers */
ssize_t Sio_puts(char s[]);
ssize_t Sio_putl(long v);
void Sio_error(char s[]);

/* Unix I/O wrappers */
int Open(const char *pathname, int flags, mode_t mode);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
off_t Lseek(int fildes, off_t offset, int whence);
void Close(int fd);
int Select(int  n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
	   struct timeval *timeout);
int Dup2(int fd1, int fd2);
void Stat(const char *filename, struct stat *buf);
void Fstat(int fd, struct stat *buf) ;

/* Directory wrappers */
DIR *Opendir(const char *name);
struct dirent *Readdir(DIR *dirp);
int Closedir(DIR *dirp);

/* Memory mapping wrappers */
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void Munmap(void *start, size_t length);

/* Standard I/O wrappers */
void Fclose(FILE *fp);
FILE *Fdopen(int fd, const char *type);
char *Fgets(char *ptr, int n, FILE *stream);
FILE *Fopen(const char *filename, const char *mode);
void Fputs(const char *ptr, FILE *stream);
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/* Dynamic storage allocation wrappers */
void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
void *Calloc(size_t nmemb, size_t size);
void Free(void *ptr);

/* Sockets interface wrappers */
int Socket(int domain, int type, int protocol);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void Listen(int s, int backlog);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

/* Protocol independent wrappers */
void Getaddrinfo(const char *node, const char *service, 
                 const struct addrinfo *hints, struct addrinfo **res);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
                 size_t hostlen, char *serv, size_t servlen, int flags);
void Freeaddrinfo(struct addrinfo *res);
void Inet_ntop(int af, const void *src, char *dst, socklen_t size);
void Inet_pton(int af, const char *src, void *dst); 

/* DNS wrappers */
struct hostent *Gethostbyname(const char *name);
struct hostent *Gethostbyaddr(const char *addr, int len, int type);

/* Pthreads thread control wrappers */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp, 
		    void * (*routine)(void *), void *argp);
void Pthread_join(pthread_t tid, void **thread_return);
void Pthread_cancel(pthread_t tid);
void Pthread_detach(pthread_t tid);
void Pthread_exit(void *retval);
pthread_t Pthread_self(void);
void Pthread_once(pthread_once_t *once_control, void (*init_function)());

/* POSIX semaphore wrappers */
void Sem_init(sem_t *sem, int pshared, unsigned int value);
void P(sem_t *sem);
void V(sem_t *sem);

/* Rio (Robust I/O) package */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd); 
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Wrappers for Rio package */
ssize_t Rio_readn(int fd, void *usrbuf, size_t n);
void Rio_writen(int fd, void *usrbuf, size_t n);
void Rio_readinitb(rio_t *rp, int fd); 
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Reentrant protocol-independent client/server helpers */
int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);

/* Wrappers for reentrant protocol-independent client/server helpers */
int Open_clientfd(char *hostname, char *port);
int Open_listenfd(char *port);


#endif /* __CSAPP_H__ */
/* $end csapp.h */
