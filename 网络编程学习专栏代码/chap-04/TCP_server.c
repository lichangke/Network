#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zconf.h>
#include <errno.h>

#define SERVER_PORT 12345


size_t readn(int fd, void *buffer, size_t size){
    char *buffer_pointer = buffer;      // buffer
    int length = size;  // 需循环接受数据大小

    while(length > 0){  // 没读满 size 个字节之前，一直都要循环下去

        int result = read(fd, buffer_pointer, length);

        if (result < 0) { 

            if (errno == EINTR) 
            {
                continue; /* 考虑非阻塞的情况，这里需要再次调用read */ 
            }
            else 
            {
                return (-1); 
            }
            
        } else if (result == 0)     // 读到对方发出的 FIN 包，表现形式是 EOF，此时需要关闭套接字
            break; /* EOF(End of File)表示套接字关闭 */
        length -= result; 
        buffer_pointer += result;
    }
    return (size - length); /* 读取 EOF 跳出循环后，返回实际读取的字符数。 返回的是实际读取的字节数*/
}

void read_data(int sockfd) {
    ssize_t n;
    char buf[1024];

    int time = 0;
    for (;;) {
        fprintf(stdout, "block in read\n");
        if ((n = readn(sockfd, (void *) buf, (size_t) 1024)) == 0)
            return;
        time++;
        fprintf(stdout, "1K read for %d \n", time);
        sleep(10);
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
	// 创建监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // 套接字地址格式
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);
	/* bind到本地地址，端口为12345 */
    bind(listenfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    /* listen的backlog为1024 */
    listen(listenfd, 1024);
	/* 循环处理用户请求 */
    for (;;) {
        clilen = sizeof(cliaddr);
        // accept 获取实际的连接
        connfd = accept(listenfd, (const struct sockaddr *) &cliaddr, &clilen);
        read_data(connfd);	/* 读取数据 */
        close(connfd);	/* 关闭连接套接字，注意不是监听套接字*/
    }
    return 0;
}



