#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/libfs.h"

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Usage : mklibfs <size> <inode>\n");
        return 1;
    }
    FILE* dsk_libfs;
    if(dsk_libfs = fopen("dsk-libfs.bin", "r")){
        fclose(dsk_libfs);
        printf("Partition file already exists.\n");
        return 1;
    }
    if(atoi(argv[1]) < atoi(argv[2])){
        printf("Need at least 1 block per inode.\n");
        return 1;
    }
    dsk_libfs = fopen("dsk-libfs.bin", "w");

    //initialize and write super block to partition
    unsigned int block_count = atoi(argv[1]);
    unsigned int inode_count = atoi(argv[2]);
    struct filsys* sb = malloc(sizeof(struct filsys));
    sb->s_fsize = block_count;
    sb->s_isize = inode_count;
    sb->s_ninode = NIFREE - 1;
    for(int i = 0; i < NIFREE; i++){
        sb->s_inode[i] = i;
    }
    int a = fwrite(sb, sizeof(struct filsys), 1, dsk_libfs);

    //write free block bitmap, block 0 used by root inode
    uint8_t* b_bitmap = malloc(block_count);
    memset(b_bitmap, 0, block_count);
    b_bitmap[0] = 1;
    int b = fwrite(b_bitmap, block_count, 1, dsk_libfs);

    //write inode table
    struct dinode dinode = {
        .di_addr = {0},
        .di_mode = 0, //mode 0 = free inode
        .di_size = 0
    };
    struct dinode root = {
        .di_addr = {0},
        .di_mode = IFDIR|IREAD|IWRITE,
        .di_size = 0
    };
    fwrite(&root, sizeof(struct dinode), 1, dsk_libfs);
    for(int i = 0; i<inode_count; ++i)
        fwrite(&dinode, sizeof(struct dinode), 1, dsk_libfs);

    //write block table
    uint8_t* blocks = malloc(BSIZE*block_count);
    fwrite(blocks, BSIZE*block_count, 1, dsk_libfs);
    fclose(dsk_libfs);
    return 0;
}
