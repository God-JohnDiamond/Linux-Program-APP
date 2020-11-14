/*
 * @Author: John Diamond
 * @Date: 2020-11-10 15:04:12
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-11-12 14:50:12
 * @FilePath: /app/07_freetype/01_wchar/ft_test.c
 */
#include <stdio.h>
#include <string.h>
#include <wchar.h>


int main( int argc, char** argv)
{
	wchar_t *chinese_str = L"中";
	unsigned int *p = (wchar_t *)chinese_str;
	int i;

	printf("sizeof(wchar_t) = %d, str's Uniocde: \n", (int)sizeof(wchar_t));
	for (i = 0; i < wcslen(chinese_str); i++)
	{
		printf("0x%x ", p[i]);
	}
	printf("\n");

	return 0;
}
