#include "../include/diskio.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//read block from disk
void bread(int blkno, void* b){
    fseek(dsk_libfs, (BSIZE*blkno)+blocks_offset, SEEK_SET);
    fread(b, BSIZE, 1, dsk_libfs);
}

//write a block's contents to disk
void bwrite(int blkno, void* b){
    fseek(dsk_libfs, (BSIZE*blkno)+blocks_offset, SEEK_SET);
    fwrite(b, BSIZE, 1, dsk_libfs);
}

//read free block bitmap
void rd_bitmap(uint8_t* bitmap){
    fseek(dsk_libfs, bitmap_offset, SEEK_SET);
    fread(bitmap, block_count, 1, dsk_libfs);
}

//save free block bitmap
void wr_bitmap(uint8_t* bitmap){
    fseek(dsk_libfs, bitmap_offset, SEEK_SET);
    fwrite(bitmap, block_count, 1, dsk_libfs);
}