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
#include "help.h"

# define SECTOR 512
// Please remember to give write permission to your argv[1] (the image you want map) by using chmod (if it doest not have the write permission by default), otherwise you will fail the map.

//get the OsName from the boot sector
//but shown in the manual, but it start from the byte 3, length 8
void getOsName(char *list, char *p, char *sample) {
	p += 3;
	for (int i = 0; i < 8; i++) {
		list[i] = p[i ];
	}
	printf("OS name: %s\n", list);
}

//normally, the lable start from the 43th, 8 length, but in this assignment, if the first character is ' ', 
//go to the root directory. traverce all entries, 
//till we find the number that the 3rd bit is set, which means 0x80(from the manual)
void getLable(char *list, char *p) {
	for (int i = 0; i < 8; i ++) {
		list[i] = p[i + 43];
	}
	if (list[0] == ' ') {
		p += SECTOR * 19;
		while (p[0] != 0x00) {
			if (p[11] == 0x08) {
				for (int i = 0; i < 8; i++) {
					list[i] = p[i];
				}
			}
			p +=32;
		}
	}
	printf("Lable of the disk: %s\n", list);
}

//a helper function to find the number of sectors.
//the 19th and 20th stand for sec number.(little endian)
int getSecCount(char *p) {
	int secCount;
	int highSC = p[20] << 8;
	int lowSC = p[19];
	secCount = highSC + lowSC;
	return secCount;
}

//according to the number of sectors, subtract previous 33,
//the result is the number of data setors, according to this, find each FAT entry, decide if it is 0x00(free)
 int getFreeSize(char *p) {
	int countFreeSectors = 0;
	for(int i = 0; i < getSecCount(p) - 33; i ++) {
		if (getFatEntry(i, p) == 0x00) {
			countFreeSectors ++;
		}
	}
	return countFreeSectors * 512;
	
}

//Count the # of files = # of directory entries in each directory.
//each entry is 32 bytes,
//count +1 if it is a subdirectory, add constrains according to the manual.

int countFiles(char *p) {
	p += SECTOR * 19;
	int countFiles = 0;
	while (p[0] != 0x00) {
		int high = p[27];
		int low = p[26];
		int firstLogicalCluster = (high << 8) + low;
		//if the 0x10 is set, this is a subdirectory, ignore it and keep traverse. 
		//skip a directory entry according to constrains
		if ((p[11] & 0x10) != 0X10 && (p[11] != 0x0f) && (firstLogicalCluster != 0) && (firstLogicalCluster != 1) && (p[11]&0x08) !=0x08 && (p[0] !=0xe5) ){
			countFiles ++;
		}
		p += 32;//traverse all directory entries, each of which is 32 bytes long
	}
	return countFiles;
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
	
	
	//get OS name
	char *sample = malloc(sizeof(char));
	char *osname = malloc(sizeof(char));
	getOsName(osname, p, sample);
	
	//get Lable
	char *lable = malloc(sizeof(char));
	getLable(lable, p);
	
	//get total size
	printf("Total size of the disk: %d bytes", getTotalSize(p));
	printf("bytes\n");
	
	//get free size
	printf("Free size of the disk: %d ", getFreeSize(p));
	printf("bytes\n");
	
	printf("=========================\n");
	printf("num of files: %d\n", countFiles(p));
	printf("=========================\n");
	
	//get number of FAT copies
	printf("Number of FAT copies: %d\n", p[16]);
	
	//get sector per FAT
	printf("Sector per FAT: %d\n", (p[23] << 8) + p[22]);
	
	
	//p[0] = 0x00; // an example to manipulate the memory, be careful when you try to uncomment this line because your image will be modified
	
	// printf("modified the memory\n");
	free(osname);
	free(sample);
	free(lable);
	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}