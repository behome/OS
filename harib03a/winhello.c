int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_closewin(int win);
int api_getkey(int mode);
void api_end(void);


void HariMain(void)
{
	int win;
	char *buf;
	api_initmalloc();
	buf = api_malloc(160*100);
	win = api_openwin(buf, 160, 100, -1, "problem");
	api_boxfilwin(win, 4, 24, 155, 95, 0 /* 崟 */);
	x = 76;
	y = 56;
	api_putstrwin(win, x, y, 3 /* 墿 */, 1, buf);
	api_closewin(win);
	api_end();
}
