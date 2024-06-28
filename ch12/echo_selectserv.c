#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUF_SIZE 100 // 定义缓冲区大小为100字节

void error_handling(char *message); // 错误处理函数声明

int main(int argc, char *argv[]) {
  int serv_sock, clnt_sock;              // 服务端和客户端套接字
  struct sockaddr_in serv_adr, clnt_adr; // 服务端和客户端地址结构体
  struct timeval timeout;                // 超时时间结构体
  fd_set reads, cpy_reads;               // 文件描述符集合

  socklen_t adr_sz; // 地址大小
  int fd_max, str_len, fd_num,
      i; // 最大文件描述符、字符串长度、文件描述符数量和循环变量
  char buf[BUF_SIZE]; // 缓冲区

  // 检查命令行参数
  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
    exit(1);
  }

  // 创建服务端套接字
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1) {
    error_handling("socket() error");
  }

  // 初始化服务端地址结构体
  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_adr.sin_port = htons(atoi(argv[1]));

  // 绑定服务端地址
  if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) {
    error_handling("bind() error");
  }

  // 开始监听
  if (listen(serv_sock, 5) == -1) {
    error_handling("listen() error");
  }

  // 初始化文件描述符集合
  FD_ZERO(&reads);           // 清空集合
  FD_SET(serv_sock, &reads); // 将服务端套接字加入集合
  fd_max = serv_sock;        // 初始化最大文件描述符

  while (1) {
    cpy_reads = reads;      // 复制文件描述符集合
    timeout.tv_sec = 5;     // 设置超时时间为5秒
    timeout.tv_usec = 5000; // 设置微秒部分为5000

    // 监视文件描述符集合
    fd_num = select(fd_max + 1, &cpy_reads, NULL, NULL, &timeout);
    if (fd_num == -1) { // 发生错误
      error_handling("select() error");
      break;
    }
    if (fd_num == 0) { // 超时，无文件描述符变化
      continue;
    }

    // 遍历文件描述符
    for (i = 0; i < fd_max + 1; i++) {
      if (FD_ISSET(i, &cpy_reads)) { // 查找发生变化的文件描述符
        if (i == serv_sock) {        // 处理服务端套接字
          adr_sz = sizeof(clnt_adr);
          clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
          if (clnt_sock == -1) {
            error_handling("accept() error");
          }
          FD_SET(clnt_sock, &reads); // 将客户端套接字加入集合
          if (fd_max < clnt_sock) {
            fd_max = clnt_sock; // 更新最大文件描述符
          }
          printf("Connected client: %d \n", clnt_sock); // 输出连接的客户端
        } else {                               // 处理客户端套接字
          str_len = read(i, buf, BUF_SIZE);    // 从客户端读取数据
          if (str_len == 0) {                  // 客户端关闭连接
            FD_CLR(i, &reads);                 // 从集合中删除该套接字
            close(i);                          // 关闭套接字
            printf("closed client: %d \n", i); // 输出关闭的客户端
          } else {
            write(i, buf, str_len); // 回送数据到客户端
          }
        }
      }
    }
  }

  close(serv_sock); // 关闭服务端套接字
  return 0;
}

// 错误处理函数
void error_handling(char *message) {
  fputs(message, stderr); // 输出错误信息到标准错误
  fputc('\n', stderr);    // 输出换行符
  exit(1);                // 退出程序
}
