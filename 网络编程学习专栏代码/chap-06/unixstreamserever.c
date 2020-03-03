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

    int listenfd, connfd;   // 监听套节字 连接套接字
    socklen_t clilen;   // 客户端
    struct sockaddr_un cliaddr, servaddr;
    
    // 本地套接字， 字节流
    listenfd = socket(AF_LOCAL, SOCK_STREAM, 0); 
    if (listenfd < 0) { 
        perror("socket create failed"); 
    }
    // 创建了一个本地地址
    char *local_path = argv[1]; 
    unlink(local_path);     // unlink 操作，以便把存在的文件删除掉
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sun_family = AF_LOCAL;    // 数据类型中的 sun_family 需要填写为 AF_LOCAL
    strcpy(servaddr.sun_path, local_path);

    // bind 本地套接字
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) { 
        perror("bind failed");
    }

    // listen 
    if (listen(listenfd, LISTENQ) < 0) {
        perror("listen failed");
    }

    clilen = sizeof(cliaddr);

    if ((connfd = accept(listenfd,(struct sockaddr *) &cliaddr, &clilen)) < 0) {
        perror("accept failed");
    }

    char buf[BUFFER_SIZE];


    while (1) { 
        bzero(buf, sizeof(buf)); 
        // 从套接字中按照字节流的方式读取数据
        if (read(connfd, buf, BUFFER_SIZE) == 0) { 
            printf("client quit"); 
            break; 
        } 
        printf("Receive: %s", buf); 
        char send_line[MAXLINE]; 
        sprintf(send_line, "Hi, %s", buf); 
        int nbytes = sizeof(send_line); 
        // 从套接字中按照字节流的方式发送数据
        if (write(connfd, send_line, nbytes) != nbytes) 
             perror( "write error"); 
    } 
        
    close(listenfd); 
    close(connfd); 
    exit(0);
}