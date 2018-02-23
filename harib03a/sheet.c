/*鼠标和窗口的重合处理*/

#include "bootpack.h"

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if(ctl ==0){
		goto err;
	}
	ctl->map = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
	if(ctl->map == 0){
		memman_free_4k(memman, (int) ctl, sizeof(struct SHTCTL));
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;/*一个sheet都没有*/
	for(i = 0; i < MAX_SHEETS; i++){
		ctl->sheets0[i].flags = 0;/*标记为未使用*/
		ctl->sheets0[i].ctl = ctl;/*记录所属*/
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i = 0; i < MAX_SHEETS; i++){
		if(ctl->sheets0[i].flags==0){
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;/*标记为正在使用*/
			sht->height = -1;/*隐藏*/
			sht->task = 0;
			return sht;
		}
	}
	return 0;/*所有的sheet都处于使用状态*/
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1, sid4,*p;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if(vx0 < 0){vx0 = 0;}
	if(vy0 < 0){vy0 = 0;}
	if(vx1 > ctl->xsize){vx1 = ctl->xsize;}
	if(vy1 > ctl->ysize){vy1 = ctl->ysize;}
	for(h = h0; h <= ctl->top; h++){
		sht = ctl->sheets[h];
		sid = sht-ctl->sheets0;
		buf = sht->buf;
		bx0 = vx0-sht->vx0;
		by0 = vy0-sht->vy0;
		bx1 = vx1-sht->vx0;
		by1 = vy1-sht->vy0;
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		if(bx1 > sht->bxsize){bx1 = sht->bxsize;}
		if(by1 > sht->bysize){by1 = sht->bysize;}
		if(sht->col_inv == -1){
			if((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0){
				bx1 = (bx1 - bx0)/4;/*MOV的次数*/
				sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				for(by = by0; by < by1; by++){
					vy = sht->vy0 + by;
					vx = sht->vx0 + bx0;
					p = (int *) &map[vy * ctl->xsize + vx];
					for(bx = 0; bx < bx1; bx++){
						p[bx] = sid4;
					}
				}
			}else{
				for(by = by0; by < by1; by++){
					vy = sht->vy0 + by;
					for(bx = bx0; bx < bx1; bx++){
						vx = sht->vx0 + bx;
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
			
		}else{
			for(by = by0; by < by1; by++){
				vy = sht->vy0 + by;
				for(bx = bx0; bx < bx1; bx++){
					vx = sht->vx0 + bx;
					if(buf[by * sht->bxsize + bx] != sht->col_inv){
						map[vy * ctl->xsize + vx] = sid;
					}
				}
			}
		}
		
	}
	return ;
}

//按照图层范围和坐标范围刷新图层
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1, sid4, *p, *q, i, i1, *r, bx2;
	unsigned char *buf, *vram=ctl->vram, *map = ctl->map,sid;
	struct SHEET *sht;
	/*如果refresh的范围超出了画面则修正*/
	if(vx0 < 0){vx0 = 0;}
	if(vy0 < 0){vy0 = 0;}
	if(vx1 > ctl->xsize){vx1 = ctl->xsize;}
	if(vy1 > ctl->ysize){vy1 = ctl->ysize;}
	for(h = h0; h <= h1; h++){
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht-ctl->sheets0;
		/*使用vx0~vy1，对bx0~by1进行倒推*/
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		if(bx1 > sht->bxsize){bx1 = sht->bxsize;}
		if(by1 > sht->bysize){by1 = sht->bysize;}
		if((sht->vx0 & 3) == 0){
			i = (bx0 + 3)/4;
			i1 = bx1/4;
			i1 = i1 - i;
			sid4 = sid | sid << 8 | sid << 16 | sid << 24;
			for(by = by0; by < by1; by++){
				vy = sht->vy0 + by;
				for(bx = bx0; bx < bx1 && (bx & 3) != 0; bx++){/*前面被4除多余的部分诸葛字节写入*/
					vx = sht->vx0 + bx;
					if(map[vy * ctl->xsize + vx] == sid){
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
				vx = sht->vx0 + bx;
				p = (int *) &map[vy * ctl->xsize + vx];
				q = (int *) &vram[vy * ctl->xsize + vx];
				r = (int *) &buf[by * sht->bxsize + bx];
				for(i = 0; i < i1; i++){
					if(p[i] == sid4){
						q[i] = r[i];
					}else{
						bx2 = bx + i * 4;
						vx = sht->vx0 + bx2;
						if(map[vy * ctl->xsize + vx + 0] == sid){
							vram[vy * ctl->xsize + vx + 0] = buf[by * sht->bxsize + bx2 + 0];
						}
						if(map[vy * ctl->xsize + vx + 1] == sid){
							vram[vy * ctl->xsize + vx + 1] = buf[by * sht->bxsize + bx2 + 1];
						}
						if(map[vy * ctl->xsize + vx + 2] == sid){
							vram[vy * ctl->xsize + vx + 2] = buf[by * sht->bxsize + bx2 + 2];
						}
						if(map[vy * ctl->xsize + vx + 3] == sid){
							vram[vy * ctl->xsize + vx + 3] = buf[by * sht->bxsize + bx2 + 3];
						}
					}
				}
				for(bx += i1*4; bx < bx1; bx++){/*后面被4除多余的部分逐个字节写入*/
					vx = sht->vx0 + bx;
					if(map[vy * ctl->xsize + vx] == sid){
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		}else{
			for(by = by0; by < by1; by++){
				vy = sht->vy0 + by;
				for(bx = bx0; bx < bx1; bx++){
					vx = sht->vx0 + bx;
					if(map[vy * ctl->xsize + vx] == sid){
						vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
					}
				}
			}
		}
		
	}
	return;
}
//图层高度进行设置，height为图层高度
void sheet_updown(struct SHEET *sht, int height)
{
	struct SHTCTL *ctl = sht->ctl;
	int h, old = sht->height;/*存储设置前的高度信息*/
	
	/*如果指定的高度过高或过低，则进行修正*/
	if(height > ctl->top+1){
		height = ctl->top+1;
	}
	if(height <-1){
		height = -1;
	}
	sht->height = height;/*设定高度*/
	
	/*下面主要是进行sheet[]的重新排列*/
	if(old > height){/*比以前低*/
		if(height >= 0){
			/*把中间的往上提*/
			for(h = old; h > height; h--){
				ctl->sheets[h] = ctl->sheets[h-1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		}else{/*隐藏*/
			if(ctl->top > old){
				/*把上面的都降下来*/
				for(h = old; h < ctl->top; h++){
					ctl->sheets[h] = ctl->sheets[h+1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;/*由于显示中的图层减少了一个，所以上面的图层高度降下来*/
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
	}else if(old<height){/*比以前的高*/
		if(old >= 0){
			/*把中间的拉下去*/
			for(h = old; h<height; h++){
				ctl->sheets[h] = ctl->sheets[h+1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}else {/*由隐藏状态转为显示状态*/
			/*将已经在上面的提上来*/
			for(h = ctl->top; h>=height; h--){
				ctl->sheets[h+1] = ctl->sheets[h];
				ctl->sheets[h+1]->height = h+1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;/*由于已显示的图层多了一个，所以最上面的图层高度增加*/
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}
//刷新图层部分内容，bx0表示图层需要刷新的部分的内部起始x坐标，by0表示内部起始y坐标，bx1，bx2则分别表示需要刷新部分的结束坐标

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if(sht->height >= 0){/*如果正在显示，则按新图层刷新画面*/
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1,sht->height, sht->height);
	}
	return;
}

//图层滑动不进行高度变换调用此函数，vx0代表该图层现在的x坐标，vy0代表图层现在的y坐标
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	struct SHTCTL *ctl = sht->ctl;
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->height >= 0){/*如果正在显示,按新图层刷新画面*/
		sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}

void sheet_free(struct SHEET *sht)
{
	if(sht->height >= 0){
		sheet_updown(sht, -1);/*如果处于显示状态，则先设定为隐藏*/
	}
	sht->flags = 0;/*标记为未使用*/
	return;
}
