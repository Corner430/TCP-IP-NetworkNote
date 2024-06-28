#include <arpa/inet.h>  // 包含网络相关函数定义
#include <stdio.h>      // 包含标准输入输出函数定义
#include <stdlib.h>     // 包含通用工具函数定义
#include <string.h>     // 包含字符串操作函数定义
#include <sys/epoll.h>  // 包含 epoll 函数定义
#include <sys/socket.h> // 包含套接字函数定义
#include <unistd.h>     // 包含 POSIX 标准函数定义

#define BUF_SIZE 100                // 缓冲区大小
#define EPOLL_SIZE 50               // epoll 事件大小
void error_handling(char *message); // 错误处理函数声明

int main(int argc, char *argv[]) {
  int serv_sock, clnt_sock;              // 服务端和客户端套接字
  struct sockaddr_in serv_adr, clnt_adr; // 服务端和客户端地址结构体
  socklen_t adr_sz;                      // 地址大小
  int str_len, i;                        // 字符串长度和循环变量
  char buf[BUF_SIZE];                    // 缓冲区

  struct epoll_event *ep_events; // epoll 事件数组指针
  struct epoll_event event;      // epoll 事件
  int epfd, event_cnt;           // epoll 文件描述符和事件计数

  // 检查命令行参数
  if (argc != 2) {
    printf("Usage : %s <port> \n", argv[0]);
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

  // 创建 epoll 实例
  epfd = epoll_create(EPOLL_SIZE);
  if (epfd == -1) {
    error_handling("epoll_create() error");
  }

  ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
  if (!ep_events) {
    error_handling("malloc() error");
  }

  // 初始化 epoll 事件结构体，监听服务端套接字的读事件
  event.events = EPOLLIN; // 读事件
  event.data.fd = serv_sock;

  // 将服务端套接字添加到 epoll 实例中
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event) == -1) {
    error_handling("epoll_ctl() error");
  }

  while (1) {
    // 等待事件发生
    event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
    if (event_cnt == -1) {
      puts("epoll_wait() error");
      break;
    }

    // 处理所有发生的事件
    for (i = 0; i < event_cnt; i++) {
      if (ep_events[i].data.fd == serv_sock) { // 客户端请求连接时
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
        if (clnt_sock == -1) {
          error_handling("accept() error");
        }

        // 初始化 epoll 事件结构体，监听客户端套接字的读事件
        event.events = EPOLLIN;
        event.data.fd = clnt_sock;

        // 将客户端套接字添加到 epoll 实例中
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event) == -1) {
          error_handling("epoll_ctl() error");
        }
        printf("connected client : %d \n", clnt_sock);
      } else { // 处理客户端套接字的读事件
        str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
        if (str_len == 0) { // 客户端关闭连接
          if (epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL) ==
              -1) {
            error_handling("epoll_ctl() error");
          }
          close(ep_events[i].data.fd);
          printf("closed client : %d \n", ep_events[i].data.fd);
        } else { // 回送数据到客户端
          write(ep_events[i].data.fd, buf, str_len);
        }
      }
    }
  }

  close(serv_sock); // 关闭服务端套接字
  close(epfd);      // 关闭 epoll 文件描述符
  free(ep_events);  // 释放 epoll 事件数组内存

  return 0;
}

// 错误处理函数
void error_handling(char *message) {
  fputs(message, stderr); // 输出错误信息到标准错误
  fputc('\n', stderr);    // 输出换行符
  exit(1);                // 退出程序
}
