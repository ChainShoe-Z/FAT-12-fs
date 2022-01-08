//University of Victoria
//csc 360 A3
//Chengxu Zhang
//V00024160

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<string.h>

#include "help.h"
	
#define timeOffset 14 //offset of creation time in directory entry
#define dateOffset 16 //offset of creation date in directory entry

#define SECTOR 512


// Please remember to give write permission to your argv[1] (the image you want map) by using chmod (if it doest not have the write permission by default), otherwise you will fail the map.

void print_date_time(char * directory_entry_startPos){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = *(unsigned short *)(directory_entry_startPos + timeOffset);
	date = *(unsigned short *)(directory_entry_startPos + dateOffset);
	
	//the year is stored as a value since 1980
	//the year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	day = (date & 0x1F);
	
	printf("%d-%02d-%02d ", year, month, day);
	//the hours are stored in the high five bits
	hours = (time & 0xF800) >> 11;
	//the minutes are stored in the middle 6 bits
	minutes = (time & 0x7E0) >> 5;
	
	printf("%02d:%02d\n", hours, minutes);
	
	return ;	
}


void getList(char *p) {
	char type;
	p += 512 * 19;
	int high = p[27];
	int low = p[26];		
	int firstLogicalCluster = (high << 8) + low;
	char blank = ' ';
	char *filename = malloc(sizeof(char));
	char *fileExt = malloc(sizeof(char));
	int filesize;
	while (p[0] != 0x00) { // && (p[11] & 0x02) == 0 && (p[11] & 0x08) == 0
		if ((p[11] & 0x10) == 0x10 && (p[11] != 0x0f) && (firstLogicalCluster != 0) && (firstLogicalCluster != 1) && (p[11]&0x08) !=0x08 && (p[0] !=0xe5) )  {
			type = 'D';
			filesize = 0;
		}
		else if ((p[11] & 0x10) == 0x00 && (p[11] != 0x0f) && (firstLogicalCluster != 0) && (firstLogicalCluster != 1) && (p[11]&0x08) !=0x08 && (p[0] !=0xe5) )  {
			type = 'F';
			filesize = p[28] + ((p[29]&0xff) << 8) + ((p[30]&0xff) << 16) + ((p[31] &0xff)<< 24);
		}
		
		for (int i = 0; i < 8; i ++) {
			if (p[i] != ' ') {
				filename[i] = p[i];
			}
			else {
				break;
			}
		}
		
		//p += 8;
		
		for (int i = 0; i < 3; i ++) {
			fileExt[i] = p[i+8];
		}
		
		strcat(filename, ".");
		strcat(filename, fileExt);
		
		printf("%c%c", type, blank);
		printf("%-20d ", filesize);
		printf("%-20s%c", filename, blank);
		print_date_time(p + 19);
		p += 32;
	}
	free(filename);
	free(fileExt);
}

int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);
	printf("Size: %lu\n\n", (uint64_t)sb.st_size);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
		
	getList(p);
	
	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}
