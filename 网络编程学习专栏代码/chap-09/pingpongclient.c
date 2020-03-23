#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>

#include "message_struct.h"

#define SERV_PORT 12345
#define MAXLINE 4096
#define KEEP_ALIVE_TIME 10          // 保活时间
#define KEEP_ALIVE_INTERVAL 3       // 保活时间间隔
#define KEEP_ALIVE_PROBETIMES 3     // 保活探测次数


int main(int argc, char **argv) {
    if (argc !=2 ){
        error(1, 0, "usage: tcpclient ");
    }

    // 创建了 TCP 套接字；
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);    

    // 创建了 IPv4 目标地址,服务器端IP和端口
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERV_PORT); 
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // 连接服务器
    socklen_t server_len = sizeof(server_addr); 
    int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr, server_len); 
    if (connect_rt < 0) { 
        error(1, errno, "connect failed "); 
    }

    char recv_line[MAXLINE + 1]; 
    int n; 

    fd_set readmask;    // io 多路复用使用
    fd_set allreads; 
    
    struct timeval tv; 
    int heartbeats = 0; 
    tv.tv_sec = KEEP_ALIVE_TIME; 
    tv.tv_usec = 0; 
    messageObject messageObject;    // 消息变量

    FD_ZERO(&allreads);      /*将allreads清零使集合中不含任何fd*/
    FD_SET(socket_fd, &allreads);   /*将allreads的第socket_fd位置1 将socket_fd加入allreads集合*/

    for (;;) {
        readmask = allreads;    // 每次赋值 在select中会改变readmask的值
        // select 函数，感知 I/O 事件
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, &tv); // tv 超时时间
        if (rc < 0) {
            error(1, errno, "select failed");
        }
        if (rc == 0) {  // KEEP_ALIVE_TIME 这段时间到达之后，select 函数会返回 0

            if (++heartbeats > KEEP_ALIVE_PROBETIMES) { // 计数探活发送次数
                error(1, 0, "connection dead\n");
            }

            printf("sending heartbeat #%d\n", heartbeats);
            messageObject.type = htonl(MSG_PING);
            
            // 发送 pingpong 探活
            rc = send(socket_fd, (char *) &messageObject, sizeof(messageObject), 0);
            if (rc < 0) {
                error(1, errno, "send failure");
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL; // 保活时间间隔
            continue;
        }
        // 接收到服务器端程序的数据
        if (FD_ISSET(socket_fd, &readmask)) {

            n = read(socket_fd, recv_line, MAXLINE);

            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            printf("received heartbeat, make heartbeats to 0 \n");
            heartbeats = 0; // 刷新 保活探测次数
            tv.tv_sec = KEEP_ALIVE_TIME;    // 刷新 保活时间
        }
    }

}

