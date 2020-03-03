
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

int main(int argc, char **argv) {
    if (argc != 2) {
        perror("usage: unixstreamclient <local_path>");
    }

    int sockfd;
    struct sockaddr_un servaddr;
    // 创建了一个本地套接字
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror( "create socket failed");
    }

    // 初始化目标服务器端的地址, TCP 编程中，使用的是服务器的 IP 地址和端口作为目标，在本地套接字中则使用文件路径作为目标标识
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, argv[1]); // sun_path 这个字段标识的是目标文件路径

    // 发起对目标套接字的 connect 调用
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect failed");
    }

    char send_line[MAXLINE];
    bzero(send_line, MAXLINE);
    char recv_line[MAXLINE];

    // 从标准输入中读取字符串，向服务器端发送
    while (fgets(send_line, MAXLINE, stdin) != NULL) {

        int nbytes = sizeof(send_line);
        if (write(sockfd, send_line, nbytes) != nbytes)
            perror("write error");

        if (read(sockfd, recv_line, MAXLINE) == 0)
            perror("server terminated prematurely");

        fputs(recv_line, stdout);
    }

    exit(0);
}