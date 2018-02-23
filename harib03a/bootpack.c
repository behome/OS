/* bootpack“I•”•ª */

#include <stdio.h>
#include <string.h>
#include "bootpack.h"


#define KEYCMD_LED		0xed
void keywin_on(struct SHEET *key_win);
void keywin_off(struct SHEET *key_win);
void close_constask(struct TASK *task);
void close_console(struct SHEET *sht);


void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x00000ff0;
	struct FIFO32 fifo, keycmd;
	char s[40];
	int fifobuf[128], keycmd_buf[32];
	int mx, my, i, new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
	int j, x, y, mmx = -1, mmy = -1, mmx2 = 0;
	struct SHEET *sht = 0, *sht2;
	unsigned int memtotal;
    struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	static char keytable0[0x80] = {
		0,   '`',   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0x0a,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0,   '\\', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   '~',   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x0a,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,   '|', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,   0
	};
	unsigned char *buf_back, buf_mouse[289];
	struct SHEET *sht_back, *sht_mouse, *key_win;
	struct TASK  *task_a, *task;
	
	int key_shift = 0, key_leds = (binfo->leds >> 4)&7, keycmd_wait = -1,key_ctrl = 0,cons_current_x = 170;
	
	init_gdtidt();
	init_pic();
	io_sti();
	fifo32_init(&fifo, 128, fifobuf,0);
	fifo32_init(&keycmd, 32, keycmd_buf,0);
	init_pit();
	init_keyboard(&fifo,256);
	enable_mouse(&fifo,512,&mdec);
	io_out8(PIC0_IMR, 0xf8); /* PIC1‚ÆƒL[ƒ{[ƒh‚ð‹–‰Â(11111001) */
	io_out8(PIC1_IMR, 0xef); /* ƒ}ƒEƒX‚ð‹–‰Â(11101111) */
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); 
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	
	/*sht_back*/
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);/*–v—L“§–¾F*/
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	
	/* sht_cons */
	key_win = open_console(shtctl, memtotal);

	/*sht_mouse*/
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 17,17,99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 17) / 2; /* ‰æ–Ê’†‰›¿??ŽZ */
	my = (binfo->scrny - 28 - 17) / 2;
	
	
	sheet_slide(sht_back, 0, 0);
	sheet_slide(key_win, 100, 100);
	
	
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,     0);
	sheet_updown(key_win,  1);
	
	sheet_updown(sht_mouse,    2);
	keywin_on(key_win);
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);
	
	*((int *) 0x0fe4) = (int) shtctl;
	*((int *) 0x0fec) = (int) &fifo;
	
	for (;;) {
		if(fifo32_status(&keycmd)>0 && keycmd_wait < 0){
			/*”@‰Ê‘¶ÝŒü???‘—“I”˜C??‘—›€*/
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if(fifo32_status(&fifo) == 0){
			if(new_mx >= 0){
				io_sti();
				sheet_slide(sht_mouse, new_mx, new_my);
				new_mx = -1;
			}else if(new_wx != 0x7fffffff){
				io_sti();
				sheet_slide(sht, new_wx, new_wy);
				new_wx = 0x7fffffff;
			}else{
				task_sleep(task_a);
				io_sti();
			}	
		}else{
			i = fifo32_get(&fifo);
			io_sti();
			if(key_win != 0 && key_win->flags == 0){/*?“üâxŒû”í??*/
				if(shtctl->top == 1){//当画面只剩鼠标和背景
					key_win = 0;
				}else{
					key_win = shtctl->sheets[shtctl->top-1];
					keywin_on(key_win);
				}	
			}
			if(256 <= i && i <= 511){/*??”˜*/
				if(i < 0x80 + 256){/*«ˆÂ?????¬Žš•„??*/
					if(key_shift == 0){
						s[0] = keytable0[i-256];
					 }else{
						s[0] = keytable1[i - 256];
					 }
				  }else{
					  s[0] = 0;
				  }
				if('A' <= s[0]&& s[0] <= 'Z'){
					if(((key_leds & 4)==0 && key_shift == 0)||((key_leds & 4)!=0 && key_shift != 0)){
						s[0]+=0x20;
					}
				}
				if(s[0] != 0 && key_win != 0 && key_ctrl == 0){/*一般字符、退格键和回车键*/
					fifo32_put(&key_win->task->fifo, s[0] + 256);
				}
				if(i == 256 + 0x0f && key_win != 0){/*Tab键*/
						keywin_off(key_win);
						j = key_win->height - 1;
						if(j == 0){
							j = shtctl->top - 1;
						}
						key_win = shtctl->sheets[j];
						keywin_on(key_win);
				}
				
				if(i == 256 + 0x2a){
					key_shift |= 1;
				}
				if(i == 256 + 0x36){
					key_shift |= 2;
				}
				if(i == 256 + 0xaa){
					key_shift &= ~1;
				}
				if(i == 256 + 0xb6){
					key_shift &= ~2;
				}
				if(i == 256 + 0x1d){
					key_ctrl |= 1;
				}
				if(i == 256 + 0x9d){
					key_ctrl &= ~1;
				}
				if(i == 256 + 0x3a){/*CapsLock*/
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if(i == 256 + 0x45){/*NumLock*/
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if(i == 256 + 0x46){/*ScrollLock*/
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if(i == 256 + 0xfa){/*??¬Œ÷Ú¾”˜*/
					keycmd_wait = -1;
				}
				if(i == 256 + 0xfe){/*??–v—L¬Œ÷Ú¾”˜*/
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if(i == 256 + 0x5b && shtctl->top > 2){//windows键
					sheet_updown(shtctl->sheets[1], shtctl->top-1);
				}
				if(i == 256 + 0x38 && key_win != 0){//alt键强制结束应用程序
					task = key_win->task;
					if(task != 0 && task->tss.ss0 != 0){
						cons_putstr0(task->cons,"\nBreak!\n");
						io_cli();
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						task_run(task, -1, 0);/*为了确实执行结束任务， 如果处于休眠状态则唤醒*/
					}
					
				}
				if(i == 256 + 0x31 && key_ctrl != 0 ){//ctrl + N 打开新的命令行窗口
					if(key_win != 0){
						keywin_off(key_win);
					}
					key_win = open_console(shtctl, memtotal);
					sheet_slide(key_win, 170, 100);
					//cons_current_x += 70;
					sheet_updown(key_win, shtctl->top);
					keywin_on(key_win);
				}
			}else if(512 <= i && i <= 767){/*鼠标数据˜*/
				if(mouse_decode(&mdec,i-512)!=0){
					mx+=mdec.x;
					my+=mdec.y;
					if(mx<0){
						mx=0;
					}
					if(my<0){
						my=0;
					}
					if(mx>binfo->scrnx-1){
						mx=binfo->scrnx-1;
					}
					if(my>binfo->scrny-1){
						my=binfo->scrny-1;
					}
					new_mx = mx;
					new_my = my;
					if((mdec.btn&0x01)!=0){/*按下鼠标左键*/
						if(mmx < 0){/*’Êí–ÍŽ®*/
							/*ˆÂÆ˜¸ã“ž‰º“I?˜?Q‘l?ŠŽwŒü“I??*/
							for(j = shtctl->top - 1; j > 0; j--){
								sht = shtctl->sheets[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if(0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize){
									if(sht->buf[y * sht->bxsize + x] != sht->col_inv){
										sheet_updown(sht, shtctl->top-1);
										if(sht != key_win){
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										if(3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21){
											mmx = mx;
											mmy = my;
											mmx2 = sht->vx0;
											new_wy = sht->vy0;
										}
										if(sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19){
											/*点击关闭X*/
											if((sht->flags & 0x10) != 0){/*判断是不是应用程序*/
												task = sht->task;
												cons_putstr0(task->cons, "\nBreak(mouse):\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int) asm_end_app;
												io_sti();
												task_run(task, -1, 0);
											}else{/*命令行窗口*/
												task = sht->task;
												sheet_updown(sht, -1);
												keywin_off(key_win);
												key_win = shtctl->sheets[shtctl->top-1];
												keywin_on(key_win);
												io_cli();
												fifo32_put(&task->fifo, 4);
												io_sti();
											}
										}
										break;
									}
									
								}
							}
						}else{
							/*如果处于窗口移动模式*/
							x = mx - mmx;
							y = my - mmy;/*计算鼠标指针的位移量*/
							new_wx = (mmx2 + x + 2) & ~3;
							new_wy = new_wy + y;
							mmy = my;/*更新到移动后的坐标*/
						}
						
					}else{
						/*没有按下鼠标左键*/
						mmx = -1;/*切换到一般模式*/
						if(new_wx != 0x7fffffff){
							sheet_slide(sht, new_wx, new_wy);
							new_wx = 0x7fffffff;
						}
					}
				}
			}else if(768 <= i && i <= 1023){/*命令行关闭操作*/
				close_console(shtctl->sheets0 + (i-768));
			}else if(1024 <= i && i <= 2023){
				close_constask(taskctl->tasks0 + (i - 1024));
			}else if(2024 <= i && i <= 2279){//只关闭命令行窗口
				sht2 = shtctl->sheets0 + (i - 1024);
				memman_free_4k(memman, (int) sht2->buf, 256 * 165);
				sheet_free(sht2);
			}
		}
	}
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle8(key_win, 1);

	if((key_win->flags & 0x20) != 0){
		fifo32_put(&key_win->task->fifo,2);/*–打开当前窗口输入 */
	}
	
	return ;
}
void keywin_off(struct SHEET *key_win)
{
	change_wtitle8(key_win,0);
	if((key_win->flags & 0x20) != 0){
		fifo32_put(&key_win->task->fifo,3);/*关闭当前窗口输入*/
	}
	return ;
}

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *)memman_alloc_4k(memman, 128 * 4);
	task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = task->cons_stack + 64*1024 -12;
	task->tss.eip = (int) &console_task;
	task->tss.es = 1*8;
	task->tss.cs = 2*8;
	task->tss.ss = 1*8;
	task->tss.ds = 1*8;
	task->tss.fs = 1*8;
	task->tss.gs = 1*8;
	*((int *) (task->tss.esp + 4)) = (int)sht;
	*((int *) (task->tss.esp + 8)) = memtotal;
	task_run(task, 2, 2);
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return task;
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	unsigned char *buf = (unsigned char *)memman_alloc_4k(memman, 256*165);
	sheet_setbuf(sht, buf, 256, 165, -1);/*无透明色*/
	make_window8(buf, 256, 165, "console", 0);
	make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
	sht->task = open_constask(sht, memtotal);
	sht->flags |= 0x20;//有光标
	return sht;
}

void close_constask(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
	task->flags = 0;/*清理当前任务*/
	return;
}
void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = sht->task;
	memman_free_4k(memman, (int)sht->buf, 256 * 16);
	sheet_free(sht);
	close_constask(task);
	return;
}
