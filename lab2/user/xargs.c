#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

//带参数列表地执行某个程序
void run(char *program, char **args) 
{
	if(fork() == 0) 
	{ 
		//子执行
		exec(program, args);
		exit(0);
	}
	return;
}

int main(int argc, char *argv[])
{
	char buf[2048]; //内存池
	char *p = buf, *last_p = buf; // 当前参数的结束、开始指针
	char *argsbuf[128]; //参数列表，包含了argv传入的参数和输入流stdin读入的参数
	char **args = argsbuf; // 指向argsbuf中第一个从输入流stdin读入的参数
	for(int i=1;i<argc;i++) 
	{
		//将argv提供的参数加入到参数列表中
		*args = argv[i];
		args++;
	}
	char **pa = args; 
	//开始从标准输入流stdin中读入参数
	while(read(0, p, 1) != 0) 
	{
		if(*p == ' ' || *p == '\n') {
			//以空格/换行符作为分隔，即读入一个参数完成
			*p = '\0';	//在内存池中，用'\0'尾零分割各个参数
			*(pa++) = last_p;
			last_p = p+1;
            //换行符，代表读入一行完成
			if(*p == '\n') 
			{
				*pa = 0; // 参数列表末尾用0标识列表结束
				run(argv[1], argsbuf); //执行当前一行指令
				pa = args; //重置读入参数指针
			}
		}
		p++;
	}
	if(pa != args) 
	{ 
		// 如果最后一行不是空行
		*p = '\0';
		*(pa++) = last_p;
		*pa = 0; // 参数列表末尾用0标识列表结束
		// 执行最后一行指令
		run(argv[1], argsbuf);
	}
	while(wait(0) != -1) {}; // 循环等待所有子进程完成
	exit(0);
}