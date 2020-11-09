/*
 * @Author: John Diamond
 * @Date: 2020-10-09 14:22:35
 * @LastEditors: John Diamond
 * @LastEditTime: 2020-10-09 17:15:37
 * @FilePath: \undefinedd:\sftp\app\led\syscall\led_sys.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define LED_DEV_PATH	"/sys/class/leds/status_led/brightness"

int main(int argc, char *argv[])
{
	int fd;

	fd = open(LED_DEV_PATH, O_RDWR);
	if(fd < 0)
	{
		printf("open failed\n");
		return -1;
	}
	
	for(;;)
	{
		write(fd, "255", 1);
		sleep(1);
		
		write(fd, "0", 1);
		sleep(1);
	}
}
