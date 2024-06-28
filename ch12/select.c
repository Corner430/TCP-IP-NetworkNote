#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define BUF_SIZE 30

int main(int argc, char *argv[]) {
  fd_set reads, temps; // 定义文件描述符集合
  int result, str_len; // 用于存储 select 的返回值和读取的字符串长度
  char buf[BUF_SIZE];     // 用于存储读取的字符串
  struct timeval timeout; // 定义超时时间结构体

  FD_ZERO(&reads); // 初始化文件描述符集合，将所有位设为 0
  FD_SET(0, &reads); // 将文件描述符0（标准输入）加入集合，将其对应的位设为1

  while (1) {
    temps = reads; // 临时变量，用于保存reads的值，以防select函数改变集合内容
    timeout.tv_sec = 5;  // 设置超时时间为5秒
    timeout.tv_usec = 0; // 设置微秒部分为0

    // 调用select函数，监视文件描述符0（标准输入）
    // 第一个参数指定需要监视的文件描述符范围，应该是集合中最大文件描述符+1，这里是1
    // 第二个参数是读集合，第三个参数是写集合，这里传递NULL
    // 第四个参数是异常集合，这里传递NULL，第五个参数是超时时间
    result = select(1, &temps, NULL, NULL, &timeout);

    // 检查 select 函数的返回值
    if (result == -1) { // 如果返回 -1，表示发生错误
      puts("select error!");
      break;
    } else if (result == 0) { // 如果返回 0，表示超时
      puts("Time-out!");
    } else {
      // 检查文件描述符0是否在temps集合中
      if (FD_ISSET(0, &temps)) { // 如果文件描述符 0 有变化，表示标准输入有数据
        str_len = read(0, buf, BUF_SIZE); // 从标准输入读取数据
        buf[str_len] = 0; // 在读取的字符串末尾加上终止符
        printf("message from console: %s", buf); // 打印读取到的数据
      }
    }
  }
  return 0;
}
