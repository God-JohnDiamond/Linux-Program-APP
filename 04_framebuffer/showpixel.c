/*
 * @Author: John Diamond
 * @Date: 2020-11-02 11:20:09
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-11-03 14:44:02
 * @FilePath: /app/framebuffer/framebuffer.c
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

static int fd;
static struct fb_var_screeninfo var;
static unsigned long ScreenSize;
static unsigned char *fb_base;
static unsigned int line_width;


void lcd_put_pexel(unsigned int x, unsigned int y, unsigned int color)
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

int main(int argc, char *argv[])
{
	int i, j;
	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0)
	{
		printf("open failed \n");
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &var))
	{
		printf("get var failed \n");
	}
	ScreenSize = var.xres * var.yres * var.bits_per_pixel / 8;
	line_width = var.xres * var.bits_per_pixel / 8;

	fb_base = (unsigned char *)mmap(NULL, ScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb_base == (unsigned char *)-1)
	{
		printf("mmap failed \n");
	}
	memset(fb_base, 0xFF, ScreenSize);
	printf("%d %d %d \n", var.xres, var.yres, var.bits_per_pixel);
	for(i = 0; i < 50; i++)
		for(j = 0; j < 100; j++)
			lcd_put_pexel(var.xres/2+j, var.yres/2+i, 0x0000FF + 10*j);
	
	munmap(fb_base, ScreenSize);
	close(fd);
	
}
