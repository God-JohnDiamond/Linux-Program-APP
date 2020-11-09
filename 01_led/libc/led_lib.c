/*
 * @Author: John Diamond
 * @Date: 2020-10-09 14:22:35
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-10-09 15:22:56
 * @FilePath: \undefinedd:\sftp\app\led\led_subsys.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LED_DEV_PATH	"/sys/class/leds/status_led/brightness"

int main(int argc, char *argv[])
{
	FILE *fd;

	fd = fopen(LED_DEV_PATH, "r+");
	if(fd == NULL)
	{
		printf("open failed\n");
		exit(1);
	}
	for(;;)
	{
		fwrite("255", 1 , 1, fd);
		fflush(fd);
		sleep(2);
		fwrite("0", 1, 1, fd);
		fflush(fd);
		sleep(2);
	}
	
}
