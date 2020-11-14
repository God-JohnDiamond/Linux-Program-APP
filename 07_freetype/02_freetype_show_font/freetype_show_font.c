/*
 * @Author: John Diamond
 * @Date: 2020-11-02 11:20:09
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-11-14 21:21:55
 * @FilePath: /Linux-Program-APP/07_freetype/02_freetype_show_font/freetype_show_font.c
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
#include <math.h>
#include <wchar.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

static int fd;
static struct fb_var_screeninfo var;
static unsigned long ScreenSize;
static unsigned char *fb_base;
static unsigned int line_width;

/**
 * @description: put pixel on LCD 
 * @param 坐标 x y  & color
 * @return NULL
 */
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

/**
 * @description: draw bit map of font 
 * @param 坐标 x y   
 * @return NULL
 */
void draw_bitmap(unsigned int x, unsigned int y, FT_Bitmap*  bitmap, unsigned int color)
{
	int i, j;
	unsigned int show_x, show_y;
	for(i = 0; i < bitmap->rows; i++)
	{
		for(j = 0; j < bitmap->width; j++)
		{
			if(i < 0 || j < 0 || i >= var.yres || j >= var.xres)
				continue;
			show_x = x + j;
			show_y = y + i;
			//if(*bitmap->buffer)
			if(bitmap->buffer[i * bitmap->width + j])
				lcd_put_pixel(show_x, show_y, color);
			//(bitmap->buffer) += 2;
			//printf("bitmap->buf++ addr = %d\n", bitmap->buffer++);
			//printf("&bitmap->buf[i * bitmap->width + j] addr = %d\n", &bitmap->buffer[i * bitmap->width + j]);
		}
	}
}

int main(int argc, char *argv[])
{
	int i, j;
	FT_Library library;
	FT_Face face;
	int error;
	FT_UInt font_size = 24;
	FT_GlyphSlot slot;
	wchar_t *chinese = L"福禄寿";

	if(argc == 3)
		font_size = strtoul(argv[2], NULL, 0);

	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0)
		printf("open failed \n");

	if(ioctl(fd, FBIOGET_VSCREENINFO, &var))
		printf("get var failed \n");

	ScreenSize = var.xres * var.yres * var.bits_per_pixel / 8;
	line_width = var.xres * var.bits_per_pixel / 8;

	fb_base = (unsigned char *)mmap(NULL, ScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb_base == (unsigned char *)-1)
		printf("mmap failed \n");

	memset(fb_base, 0, ScreenSize);

	error = FT_Init_FreeType(&library);
	if(error)
		printf("init FT failed\n");

	error = FT_New_Face(library, argv[1], 0, &face);
	if(error == FT_Err_Unknown_File_Format)
		printf("open font file success but its font format is unsupported\n");
	else if(error)
		printf("open font file failed\n");

	slot = face->glyph;

	FT_Set_Pixel_Sizes(face, font_size, 0);
	if(error)
		printf("set font size failed \n");
	
	for(i = 0; i < wcslen(chinese); i++)
	{
		error = FT_Load_Char(face, chinese[i], FT_LOAD_RENDER);
		if(error)
			printf("load char failed \n");
		draw_bitmap(var.xres / 4 + i * font_size, var.yres / 2, &slot->bitmap, 0xff00ff);
	}

	munmap(fb_base, ScreenSize);
	close(fd);
}
