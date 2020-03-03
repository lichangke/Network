#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

# define MAXLINE 4096
static int count;

static void recvfrom_int(int signo) {
    printf("\nreveived %d datagrams\n", count);
}

int main(int argc, char **argv) {
    int socket_fd;
    // 创建 UDP 套接字
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12345);
	// 绑定到本地端口 
    bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    socklen_t client_len;
    char message[MAXLINE];
    count = 0;
	// 响应“Ctrl+C”退出时
    signal(SIGINT, recvfrom_int);
    
    struct sockaddr_in client_addr;
    client_len = sizeof(client_addr);
    for(;;) {
    	// 用 recvfrom 函数获取客户端发送的报文
        int n = recvfrom(socket_fd, message, MAXLINE, 0, (struct sockaddr *) &client_addr, &client_len);
        message[n] = 0; // 加上结束符
        printf("receive %d bytes: %s\n", n, message);
    
        char send_line[MAXLINE];
        // 加上“Hi”的前缀
        sprintf(send_line, "Hi, %s", message);
		// 通过 sendto 函数发送给客户端对端。
        sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr *) &client_addr, client_len);

        count ++;
    }
}