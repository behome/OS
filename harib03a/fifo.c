/*FIFO库*/

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size ,int *buf, struct TASK *task)
/*FIFO库的初始化*/
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p=0;
	fifo->q=0;
	fifo->task = task;
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/*数据输送到FIFO储存*/
{
	if(fifo->free == 0){
		/*没有空闲的溢出*/
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p]=data;
	fifo->p++;
	if(fifo->p == fifo->size){
		fifo->p = 0;
	}
	fifo->free--;
	if(fifo->task != 0){
		if(fifo->task->flags != 2){/*如果任务处于休眠状态*/
			task_run(fifo->task,-1,0);/*唤醒任务*/
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/*从FIFO取出1字节数据出来*/
{
	int data;
	if(fifo->free == fifo->size){
		/*缓冲区为空 返回-1*/
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if(fifo->q == fifo->size){
		fifo->q=0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/*报告数据的数据是多少*/
{
	return fifo->size-fifo->free;
}