/*
 * @Author: John Diamond
 * @Date: 2020-11-02 11:20:09
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-11-17 16:27:08
 * @FilePath: /Linux-Program-APP/07_freetype/02_freetype_show_font/freetype_show_font_line.c
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

/* calc the box size*/
void calc_Bbox(FT_Face face, FT_BBox *abbox, wchar_t *chinese)
{
	int				i;
	FT_BBox			bbox;
	FT_BBox			glyph_bbox;
	FT_GlyphSlot	slot = face->glyph;
	FT_Glyph 		glyph;
	FT_Vector		pen;
	FT_Matrix		matrix;
	double			angle;

	bbox.xMin = bbox.yMin =  32000;
	bbox.xMax = bbox.yMax = -32000;
	pen.x = 0;
	pen.y = 0;

	angle = (0 * 1.0 / 360) *3.14159 * 2;
	/* set up matrix */
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

	for(i = 0; i < wcslen(chinese); i++)
	{
		FT_Set_Transform(face, &matrix, &pen);
		FT_Load_Char(face, chinese[i], FT_LOAD_RENDER);
		FT_Get_Glyph(slot, &glyph);
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);
		/* 更新外框 范围 四条边 */
		if(glyph_bbox.xMin < bbox.xMin)
			bbox.xMin = glyph_bbox.xMin;
		if(glyph_bbox.yMin < bbox.yMin)
			bbox.yMin = glyph_bbox.yMin;
		if(glyph_bbox.xMax > bbox.xMax)
			bbox.xMax = glyph_bbox.xMax;
		if(glyph_bbox.yMax > bbox.yMax)
			bbox.yMax = glyph_bbox.yMax;

		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	printf("calc bbox xMin=%d xMax=%d yMin=%d yMax=%d \n", bbox.xMin, bbox.xMax, bbox.yMin, bbox.yMax);
	*abbox = bbox;
}

void display_string(FT_Face face, wchar_t *chinese, unsigned int lcd_x, unsigned int lcd_y)
{
	unsigned int	i, x, y;
	int				error;
	FT_GlyphSlot	slot = face->glyph;
	FT_BBox 		bbox;
	FT_Matrix		matrix;              /* transformation matrix */
	FT_Vector		pen;                 /* untransformed origin */
	double angle;
	
	/* 把LCD坐标转换为笛卡尔坐标 */
	x = lcd_x;
	y = var.yres - lcd_y;

	/* calc the whole string size */
	calc_Bbox(face, &bbox, chinese);

	/* 反推外框原点 */
	pen.x = (x - bbox.xMin) * 64;
	pen.y = (y - bbox.yMax) * 64;
	
	angle = (0 * 1.0 / 360) *3.14159 * 2;
	/* set up matrix */
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

	for(i = 0; i < wcslen(chinese); i++)
	{
		FT_Set_Transform(face, &matrix, &pen);
		error = FT_Load_Char(face, chinese[i], FT_LOAD_RENDER);
		if(error)
			printf("load char failed \n");
		draw_bitmap(slot->bitmap_left, var.yres - slot->bitmap_top, &slot->bitmap, 0xff00ee);

		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
}

int main(int argc, char *argv[])
{
	unsigned int	lcd_x, lcd_y;
	FT_Library		library;
	FT_Face 		face;
	int				error;
	FT_UInt			font_size = 50;
	wchar_t	 		*chinese = L"福禄寿.jpeg";

	if(argc == 4)
	{
		lcd_x = strtoul(argv[1], NULL, 0);
		lcd_y = strtoul(argv[2], NULL, 0);
		font_size = strtoul(argv[3], NULL, 0);
	}
	else
	{
		printf("usage is %s <lcd_x> <lcd_y> [font size]\n", argv[0]);
		return -1;
	}

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

	error = FT_New_Face(library, "simsun.ttc", 0, &face);
	if(error == FT_Err_Unknown_File_Format)
		printf("open font file success but its font format is unsupported\n");
	else if(error)
		printf("open font file failed\n");

	FT_Set_Pixel_Sizes(face, font_size, 0);
	if(error)
		printf("set font size failed \n");

	display_string(face, chinese, lcd_x, lcd_y);

	munmap(fb_base, ScreenSize);
	close(fd);
}
