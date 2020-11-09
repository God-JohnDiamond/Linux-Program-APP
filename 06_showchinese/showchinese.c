/*
 * @Author: John Diamond
 * @Date: 2020-11-03 11:20:09
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-11-09 14:14:56
 * @FilePath: /app/06_showchinese/showchinese.c
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>

#include "showchinese.h"

static int fd, fd_hzk;
static struct fb_var_screeninfo var;
static unsigned long ScreenSize;
static unsigned char *fb_base, *hzk_base;
struct stat hzk_statbuf;
static unsigned int line_width;


void lcd_put_pixel(unsigned int x, unsigned int y, unsigned int color)
{
	unsigned char	*AbAddr8 = fb_base + y * line_width + x * var.bits_per_pixel / 8;
	unsigned short	*AbAddr16 = (unsigned short *)AbAddr8;
	unsigned int	*AbAddr32 = (unsigned int *)AbAddr8;

	unsigned int red, green, blue;
	
	switch (var.bits_per_pixel)
	{
	case 8:
		*AbAddr8 = color;
		break;
	case 16: 
		red = (color >> 16) & 0xFF;
		green = (color >> 8) & 0xFF;
		blue = (color >> 0) & 0xFF;
		color = (red >> 3) << 11 | (green >> 2) << 5 | (blue >> 3);
		*AbAddr16 = color;
	case 32:
		*AbAddr32 = color;
	default:
		break;
	}
}

void lcd_put_ascii(unsigned int x, unsigned int y, unsigned char c, unsigned int color)
{
	int i, j;
	unsigned char *dot = (unsigned char *)&asciidata[(int)c * 16];
	for(i = 0; i < 16; i++)
	{
		for(j = 7; j >= 0; j--)
		{
			if((*dot >> j) & 1)
				lcd_put_pixel(x+7-j, y+i, color);
			else
				lcd_put_pixel(x+7-j, y+i, 0); // black
		}
		dot++;
	}
}

/*
 * draw a circle
 * (x,y) circle 
 * r circle r
 * 
 */
/*
void lcd_put_circle(unsigned int x, unsigned int y, float r, unsigned int color)
{
	unsigned int i, show_x, pos_show_y, neg_show_y;
	for (i = x-r; i <= x+r; i++)
	{
		show_x = i;
		pos_show_y = (unsigned int)sqrt(r*r - (show_x*show_x - x*x)) + y;
		neg_show_y = pos_show_y - 2*(pos_show_y - y);
		lcd_put_pixel(show_x, pos_show_y, color);
		lcd_put_pixel(show_x, neg_show_y, color);
	}
}
*/

void lcd_put_str(unsigned int x, unsigned int y, char *str, unsigned int color)
{
	unsigned int i, show_x, show_y;
	char *onestr;
	const char ent[2] = "\n";

	onestr = strtok(str, ent);

	while(onestr != NULL)
	{
		for(i = 0; i < strlen(onestr); i++)
		{
			show_x = x+8*i;
			show_y = y;
			lcd_put_ascii(show_x, show_y, onestr[i], color+1000*i);
		}
		onestr = strtok(NULL, ent);
		y += 16;
		//printf(" y = %d \n", show_y);
	}
}

void lcd_put_single_chinese(unsigned int x, unsigned int y, unsigned char *s, unsigned int color)
{
	int i, j, b;
	unsigned char area_code, byte_code;
	area_code = s[0] - 0xA1;
	byte_code = s[1] - 0xA1;
	unsigned char *hzk_addr = hzk_base + (area_code * 94 + byte_code)*32;

	printf("GB2312 value:\n");
	printf(" s[0]=%d\n s[1]=%d\n",s[0],s[1]);

	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 2; j++)
		{
			//printf("hzkaddr=%d\n",*hzk_addr);
			for(b = 7; b >= 0; b--)
			{
				if((*hzk_addr >> b) & 1)
					lcd_put_pixel(x+j*8+7-b, y+i, color);
				else
					lcd_put_pixel(x+j*8+7-b, y+i, 0); // black
			}
			hzk_addr++;
		}
	}
}
void lcd_put_chinese(unsigned int x, unsigned int y, unsigned char *s, unsigned int color)
{
	unsigned int i, show_x, show_y;
	char *onestr;
	const char ent[2] = "\n";

	onestr = strtok(s, ent);
	printf("s = %s\n", s);
	printf("onestr = %s\n", onestr);
	
	while(onestr != NULL)
	{
		printf("strlen(onestr) = %d\n", strlen(onestr));
		
		for(i = 0; i < strlen(onestr); i++)
		{
			if(onestr[i] > 0x7F)
			{
				show_x = x+16*i;
				show_y = y;
				lcd_put_single_chinese(show_x, show_y, &onestr[i*2], color+1000*i);
				i++;
			}
			else
			{
				show_x = x+8*i;
				show_y = y;
				lcd_put_ascii(show_x, show_y, onestr[i], color+1000*i);
			}
			
		}
		onestr = strtok(NULL, ent);
		y += 16;
		//printf(" y = %d \n", show_y);
	}
}

int main(int argc, char *argv[])
{
	unsigned char str3[200] = {"Íõ"};
	//unsigned char str2[200] = {"JohnÍõ"};
	unsigned char str2[200] = {"Íõ\nEmma\ndisplay\n"};
	unsigned char str1[200] = {"Boom! Shakalaka~"};
	unsigned char str[200] = {"Emma\nJohn\nLCD\ndisplay\n"};

	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0)
	{
		printf("open fb0 failed \n");
		return -1;
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &var))
	{
		printf("get var failed \n");
		return -1;
	}
	ScreenSize = var.xres * var.yres * var.bits_per_pixel / 8;
	line_width = var.xres * var.bits_per_pixel / 8;

	fb_base = (unsigned char *)mmap(NULL, ScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb_base == (unsigned char *)-1)
	{
		printf("mmap fb0 failed \n");
	}

	fd_hzk = open("HZK16", O_RDONLY);
	if(fd_hzk < 0)
	{
		printf("open HZK16 failed \n");
	}
	if(fstat(fd_hzk, &hzk_statbuf))
	{
		printf("get hzk infomation failed \n");
	}
	hzk_base = (unsigned char *)mmap(NULL, hzk_statbuf.st_size, PROT_READ , MAP_SHARED, fd_hzk, 0);
	if(hzk_base == (unsigned char *)-1)
	{
		printf("mmap hzk failed \n");
	}
	memset(fb_base, 0, ScreenSize); // clear screen black
	printf("%d %d %d \n", var.xres, var.yres, var.bits_per_pixel);

	//lcd_put_str(var.xres/2, var.yres/3, str1, 0xff00ff);
	//lcd_put_str(var.xres/2, var.yres/2, str, 0xff00ff);
	//lcd_put_single_chinese(var.xres/2, var.yres/3, str3, 0xff00ff);
	lcd_put_chinese(var.xres/2, var.yres/2, str2, 0xff00ff);
	//lcd_put_chinese(var.xres/2, var.yres/3, str1, 0xff00ff);

	//lcd_put_circle(var.xres/2, var.xres/2, 100, 0xff00ff);

	munmap(fb_base, ScreenSize);
	munmap(hzk_base, hzk_statbuf.st_size);
	close(fd);
	close(fd_hzk);
	return 0;
}
