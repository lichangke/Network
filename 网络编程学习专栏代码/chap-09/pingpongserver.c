#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

#include "message_struct.h"

#define SERV_PORT 12345
#define    LISTENQ        1024

static int count;

static void sig_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char **argv) {

    if (argc != 2) {
        error(1, 0, "usage: tcpsever <sleepingtime>");
    }

    int sleepingTime = atoi(argv[1]);

    int listenfd;   // 监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 绑定该套接字到本地端口和 ANY 地址上
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        error(1, errno, "bind failed ");
    }

    int rt2 = listen(listenfd, LISTENQ);
    if (rt2 < 0) {
        error(1, errno, "listen failed ");
    }

    // 信号
    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_IGN);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // accept 获取连接套接字
    if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
        error(1, errno, "bind failed ");
    }

    messageObject message;
    count = 0;


    for (;;) {

        // 读取数据
        int n = read(connfd, (char *) &message, sizeof(messageObject));
        if (n < 0) {
            error(1, errno, "error read");
        } else if (n == 0) {
            error(1, 0, "client closed \n");
        }

        printf("received %d bytes\n", n, message);
        count++;

        // 根据消息类型进行不同的处理
        switch (ntohl(message.type)) {
            case MSG_TYPE1 :
                printf("process  MSG_TYPE1 \n");
                break;

            case MSG_TYPE2 :
                printf("process  MSG_TYPE2 \n");
                break;

            case MSG_PING: {
                messageObject pong_message;
                pong_message.type = MSG_PONG;
                sleep(sleepingTime);    // 模拟服务器处理时间  
                // 回发pingpong消息 
                ssize_t rc = send(connfd, (char *) &pong_message, sizeof(pong_message), 0);
                if (rc < 0)
                    error(1, errno, "send failure");
                break;
            }

            default :
                error(1, 0, "unknown message type (%d)\n", ntohl(message.type));
        }

    }
}

