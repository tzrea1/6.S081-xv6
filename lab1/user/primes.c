#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//每一次sieve调用对应一次筛选，过程中会从pleft得到并输出一个素数p，并筛除掉p的所有倍数
//同时创建下次筛选的进程以及相应输入管道 pright，将筛选剩下的数传到下一个筛选阶段进行处理
void sieve(int pleft[2]) 
{ 
    //pleft：左端进程的输入管道
	int p;
	read(pleft[0], &p, sizeof(p)); // 读第一个数（素数）
	if(p == -1) 
    { // 如果为-1（即管道末尾标识符），则所有数字处理完毕，退出程序
		exit(0);
	}
	printf("prime %d\n", p);
	int pright[2];
	pipe(pright); //用于将筛选后的数集输出到下一进程的输出管道
	if(fork() == 0) 
    {
		//当前进程的子进程
		close(pright[1]); //子进程对其输入管道pright只需读，不需写，所以关掉写端
		close(pleft[0]); ////关闭父进程的输入管道pleft
		sieve(pright); //继续通过递归进行筛选，下一子进程以父进程的输出管道作为输入

	} 
    else {
		//当前进程（父进程）
		close(pright[0]); //对下一子进程的输入管道只需写、不需读，关闭读端
		int buf;
		while(read(pleft[0], &buf, sizeof(buf)) && buf != -1) 
        { 
            //从左端管道读入数字
			if(buf % p != 0) //筛除当前素数的倍数
            {
				write(pright[1], &buf, sizeof(buf)); //写入右端管道
			}
		}
		buf = -1;
		write(pright[1], &buf, sizeof(buf)); // 补写最后的 -1，标示输入完成。
		wait(0); //等待当前进程的子进程完成
		exit(0);
	}
}
//主进程
int main(int argc, char **argv) 
{
	int input_pipe[2];
	pipe(input_pipe); //输入管道，输入2到35间的所有整数
    if(fork() == 0) 
    {
		close(input_pipe[1]); //子进程只需读、不需写，关闭管道写端
		sieve(input_pipe);
		exit(0);
	} 
    else 
    {
		// 主进程
		close(input_pipe[0]); //关闭读端
		int i;
		for(i=2;i<=35;i++){ //生成数集[2, 35]到输入管道链的最左端
			write(input_pipe[1], &i, sizeof(int));
		}
		i = -1;
		write(input_pipe[1], &i, sizeof(int)); //末尾标-1，标识输入完成
	}
	wait(0); // 等待直接子进程（即第一个子进程）完成。
	exit(0);
}