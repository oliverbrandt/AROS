/*
    Copyright © 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: m68k-amiga ROM checksum generator
    Lang: english
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/mman.h>

static int amiga_checksum(uint8_t *mem, int size, uint32_t chkoff, int update)
{
    uint32_t oldcksum = 0, cksum = 0, prevck = 0;
    int i;

    for (i = 0; i < size; i+=4) {
	uint32_t val = (mem[i+0] << 24) + 
		       (mem[i+1] << 16) +
		       (mem[i+2] <<  8) +
		       (mem[i+3] <<  0);

    	/* Clear old checksum */
	if (update && i == chkoff) {
		oldcksum = val;
		val = 0;
	}

	cksum += val;
	if (cksum < prevck)
	    cksum++;
	prevck = cksum;
    }

    cksum = ~cksum;

    if (update && cksum != oldcksum) {
    	printf("Updating checksum from 0x%08x to 0x%08x\n", oldcksum, cksum);
	
	mem[chkoff + 0] = (cksum >> 24) & 0xff;
	mem[chkoff + 1] = (cksum >> 16) & 0xff;
	mem[chkoff + 2] = (cksum >>  8) & 0xff;
	mem[chkoff + 3] = (cksum >>  0) & 0xff;

	return 1;
   }

   return 0;
}

int main(int argc, char **argv)
{
	int err, fd;
	void *rom;
	uint32_t size = 512 * 1024;

	fd = open(argv[1], O_RDWR | O_CREAT, 0666);
	if (fd < 0) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}

	/* Resize to 512K */
	lseek(fd, size, SEEK_SET);
	lseek(fd, 0, SEEK_SET);

	rom = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	if (rom == MAP_FAILED) {
		perror(argv[1]);
		close(fd);
		return EXIT_FAILURE;
	}

	err = amiga_checksum(rom, size, 8, 0);
	err = amiga_checksum(rom, size, 8, 1);

	munmap(rom, size);

	close(fd);

	return EXIT_SUCCESS;
}


