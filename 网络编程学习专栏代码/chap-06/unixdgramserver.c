#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>  // struct sockaddr_un 
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define    MAXLINE     4096
#define    LISTENQ        1024
#define    BUFFER_SIZE    4096

int main(int argc, char **argv){
    if (argc != 2) { 
       perror("usage: unixstreamserver "); 
    }

    int socket_fd;  
    socklen_t clilen;   // 客户端
    struct sockaddr_un cliaddr, servaddr;
    
    // 本地套接字， 数据报
    socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);  // 不是 SOCK_STREAM
    if (socket_fd < 0) { 
        perror("socket create failed"); 
    }
    // 创建了一个本地地址
    char *local_path = argv[1]; 
    unlink(local_path);     // unlink 操作，以便把存在的文件删除掉
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sun_family = AF_LOCAL;    // 数据类型中的 sun_family 需要填写为 AF_LOCAL
    strcpy(servaddr.sun_path, local_path);

    // bind 本地套接字
    if (bind(socket_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) { 
        perror("bind failed");
    }

    clilen = sizeof(cliaddr);
    char buf[BUFFER_SIZE];

    while (1) { 
        bzero(buf, sizeof(buf)); 
    
        if (recvfrom(socket_fd, buf, BUFFER_SIZE, 0, (struct sockaddr *) &cliaddr, &clilen) == 0) {
            printf("client quit"); 
            break; 
        } 
        printf("Receive: %s", buf); 
        char send_line[MAXLINE]; 
        sprintf(send_line, "Hi, %s", buf); 

        size_t nbytes = strlen(send_line); 
        printf("now sending: %s \n", send_line);

        if (sendto(socket_fd, send_line, nbytes, 0, (struct sockaddr *) &cliaddr, clilen) != nbytes)
             perror( "write error"); 
    } 
        
    close(socket_fd); 
    exit(0);
}