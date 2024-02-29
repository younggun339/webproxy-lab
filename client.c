#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#define BUFFSIZE 100
#define NAMESIZE 20
char message[BUFFSIZE];
void* rcv(void* arg)
{
        int sock = (int)arg;
        char buff[500];
        int strlen = 0;
        while(1)
        {
                strlen = read(sock, buff, sizeof(buff));
                if(strlen == -1)
                {
                        printf("sock close\n");
                        break;
                }
                printf("%s\n", buff);
        }
        close(sock);
        pthread_exit(0);
        return NULL;
}
int main(int argc, char** argv)
{
        int sock;
        sock = socket(PF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        pthread_t rcv_thread;
        memset(&serv_addr, 0, sizeof(serv_addr));
        char id[100];
        strcpy(id, argv[1]);
        printf("id: %s\n", id);
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr("13.209.43.128");
        serv_addr.sin_port=htons(54321);
if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))== -1)
        {
                printf("connect error\n");
        }
        pthread_create(&rcv_thread, NULL, rcv, (void*)sock);
        char msg[1000];
        char chat[1100];
        while(1)
        {
                fgets(msg, sizeof(msg), stdin);
                //fputs(msg,stdout));
                sprintf(chat, "[%s]: %s", id, msg);
                //printf("보냈음! [%s]: %s", id, msg);
                write(sock,chat,strlen(chat) + 1);
        }
        close(sock);
        return 0;
}