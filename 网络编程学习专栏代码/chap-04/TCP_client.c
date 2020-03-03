#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MESSAGE_SIZE 10240
#define SERVER_PORT 12345
#define SERVER_ADDR "127.0.0.1"

void send_data(int);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
	// 创建客户端套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &servaddr.sin_addr);
    int connect_rt = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    if (connect_rt < 0) { 
        fprintf(stdout, "connect failed \n");
    }
    send_data(sockfd);
    return 0;
}

void send_data(int sockfd) {
    char *query;
    // 初始化了一个长度为 MESSAGE_SIZE 的字符串流
    query = malloc(MESSAGE_SIZE + 1);
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        query[i] = 'a';
    }
    query[MESSAGE_SIZE] = '\0';

    const char *cp;
    cp = query;
    long remaining = (long) strlen(query);
    while (remaining) {
    	// 调用 send 函数将 MESSAGE_SIZE 长度的字符串流发送出去
        long n_written = send(sockfd, cp, remaining, 0);
        fprintf(stdout, "send into buffer %ld \n", n_written);
        if (n_written < 0) {
            perror("send");
            return;
        }
        remaining -= n_written;	// 剩余未发送的
        cp += n_written;
    }
}