/*中断关系*/

#include "bootpack.h"
#include <stdio.h>
#define  PORT_KETDAT	0x0060
void init_pic(void)		/*PIC初始化*/
{
	io_out8(PIC0_IMR, 0xff);/*禁止所有中断*/
	io_out8(PIC1_IMR, 0xff);/*禁止所有中断*/
	
	io_out8(PIC0_ICW1, 0x11);/*边沿触发模式*/
	io_out8(PIC0_ICW2, 0x20);/*IRQ0-7由INT20-27接受*/
	io_out8(PIC0_ICW3, 1<<2);/*PIC1由IRQ2接受*/
	io_out8(PIC0_ICW4, 0x01);/*无缓冲区模式*/
	
	io_out8(PIC1_ICW1, 0x11);/*边沿触发模式*/
	io_out8(PIC1_ICW2, 0x28);/*IRQ8-15由INT28-2f接受*/
	io_out8(PIC1_ICW3, 2);/*PIC1由IRQ2接受*/
	io_out8(PIC1_ICW4, 0x01);/*无缓冲区模式*/
	
	io_out8(PIC0_IMR, 0xfb);/*PIC1以外全部禁止*/
	io_out8(PIC1_IMR, 0xff);/*禁止所有中断*/
	
	return;
}

void inthandler27(int *esp)
/*PIC从0的不完全中断对策*/
/*速龙64 X 2机等芯片组的原因PIC初期化时，这一次发生中断*/
/*这个中断处理函数，其中断对什么都不做，让过去*/
/*为什么什么也不做就好了？
这是PIC→中断初始化时的电性的噪音产生的东西，所以，
认真什么处理，不必要。*/
{
	io_out8(PIC0_OCW2, 0x67); 
	return;
}

