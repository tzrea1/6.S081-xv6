#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char **argv) {
	int p2c[2], c2p[2];
    // 管道对应的长度为2的数组，其中0存放从管道读取数据的文件描述符，1存放写入数据的文件描述符
	pipe(p2c); //父进程-子进程
	pipe(c2p); //子进程-父进程
	
	if(fork() != 0) { //父进程
		write(p2c[1], "!", 1); //父进程首先向p2c管道发出字节
		char buf;
		read(c2p[0], &buf, 1); //等待子进程
		printf("%d: received pong\n", getpid()); //通过read c2p管道，得到子进程收到数据的反馈，输出 pong
		wait(0);
	} else { //子进程
		char buf;
		read(p2c[0], &buf, 1); //读取管道，获得父进程写入到p2c管道的字节
		printf("%d: received ping\n", getpid());
		write(c2p[1], &buf, 1); //写入c2p管道，送回父进程
	}
	exit(0);
}