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
#include <string.h>




int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;
	
	if (argc < 3) {
		printf("Error: need a file\n");
		exit(1);
	}
	
	char *newChar = argv[2];
	
/* 	while (*newChar) {
			newChar = newChar - 32;
		}
        newChar++;
	}
 */			
	printf("%s\n", newChar);
	

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);
	printf("Size: %lu\n\n", (uint64_t)sb.st_size);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
	
	//p[0] = 0x00; // an example to manipulate the memory, be careful when you try to uncomment this line because your image will be modified
	
	// printf("modified the memory\n");
	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}