#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zconf.h>
#include <errno.h>
#include <signal.h>

#define MAXLINE     4096
#define SERV_PORT 12345


int main(int argc, char **argv) {

    if (argc != 2) {
        perror("usage: client ");
    }

    // 创建了一个 TCP 套接字；
    int socket_fd; 
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 连接的目标服务器 IPv4 地址，绑定到了指定的 IP 和端口；
    struct sockaddr_in server_addr; 
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERV_PORT); 
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr); 

    // 使用创建的套接字，向目标 IPv4 地址发起连接请求；
    socklen_t server_len = sizeof(server_addr); 
    int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr, server_len); 
    if (connect_rt < 0) { 
        error(1, errno, "connect failed "); 
    }

    char send_line[MAXLINE], recv_line[MAXLINE + 1]; 
    int n;

    // 使用 select 做准备，初始化描述字集合
    fd_set readmask;
    fd_set allreads;
    // 参见 https://blog.csdn.net/bailyzheng/article/details/7477446
    FD_ZERO(&allreads); /*将allreads清零使集合中不含任何fd*/
    FD_SET(0, &allreads);    /*将allreads的第0位置1，如allreads原来是00000000，则现在变为100000000，这样fd==1的文件描述字就被加进allreads中了*/
    FD_SET(socket_fd, &allreads);  /*将socket_fd加入allreads集合*/

    for (;;) {

        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);

        if (rc <= 0)
            error(1, errno, "select failed");
        //   /*测试socket_fd是否在readmask集合中*/    
        if (FD_ISSET(socket_fd, &readmask)) {
            // 连接套接字上有数据
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
         /*测试readmask的第0位是否为1*/
        if (FD_ISSET(0, &readmask)) {
            // 标准输入
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                if (strncmp(send_line, "shutdown", 8) == 0) {
                    // 将allreads的第0位置0
                    FD_CLR(0, &allreads);
                    // 调用 shutdown 函数关闭写方向
                    if (shutdown(socket_fd, 1)) {
                        error(1, errno, "shutdown failed");
                    }
                } else if (strncmp(send_line, "close", 5) == 0) {
                    FD_CLR(0, &allreads);
                    // 调用 close 函数关闭连接
                    if (close(socket_fd)) {
                        error(1, errno, "close failed");
                    }
                    sleep(6);
                    exit(0); // 退出 这里会回收一些资源
                } else {
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n') {
                        send_line[i - 1] = 0;
                    }

                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0) {
                        error(1, errno, "write failed ");
                    }
                    printf("send bytes: %zu \n", rt);
                }

            }
        }
    }


    return 0;
}



