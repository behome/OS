int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_closewin(int win);
int api_getkey(int mode);
void api_end(void);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);

void HariMain(void)
{
	char *buf;
	int win,i;
	api_initmalloc();
	buf = api_malloc(150*50);
	win = api_openwin(buf, 150, 50, -1, "hello");
	api_boxfilwin(win,  8, 36, 141, 43, 3 /* 墿 */);
	char s[60];
	sprintf(s,"%X",buf);
	api_putstrwin(win, 28, 28, 0 /* 崟 */, 12, s);
	for (;;) {
		i = api_getkey(1);
		if (i == 0x0a) { break; } /* Enter偱廔椆 */
	}	
	api_free(buf,150*50);
	api_closewin(win);
	api_end();
}
