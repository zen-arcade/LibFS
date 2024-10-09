#ifndef DISKIO_H
#define DISKIO_H
#include <stdio.h>
#include <stdint.h>
#include "libfs.h"

void bread(int blkno, void* b);
void bwrite(int blkno, void* b);
void rd_bitmap(uint8_t* bitmap);
void wr_bitmap(uint8_t* bitmap);

#endif