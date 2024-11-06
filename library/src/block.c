#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/sys.h"
#include "../include/inode.h"
#include "../include/block.h"
#include "../include/diskio.h"
#include "../include/libfs.h"


//get a free block from disk
int balloc(){
    int blkno = -1;
    uint8_t* bitmap = malloc(block_count);
    rd_bitmap(bitmap);
    for(int i = 0; i < block_count; i++){
        if(bitmap[i] == 0){
            blkno = i;
            bitmap[i] = 1;
            wr_bitmap(bitmap);
            free(bitmap);
            return blkno;
        }
    }
    printf("No more disk space!\n");
    return blkno;
}

//free a block from disk 
void bfree(int bn){
    uint8_t* bitmap = malloc(block_count);
    rd_bitmap(bitmap);
    bitmap[bn] = 0;
    wr_bitmap(bitmap);
    free(bitmap);
}

//get physical block for reading or writing (allocate new if block full)
int bmap(struct inode* ip, int bn, int flag){
    int dsk_bn = ip->i_addr[bn]; //physical block number
    int log_bn = (ip->i_size + 1) / BSIZE; //file's next or current logical block number 
    if(flag & BREAD){
        if(bn >= 5){ //look in indirect block
            unsigned int* b = malloc(BSIZE);
            bread(ip->i_addr[5], b);
            dsk_bn = b[bn - 5];
            free(b);
            return dsk_bn; 
        } else {
            return dsk_bn;
        }
    }
    //else, write access
    if(ip->i_size >= MAX_SIZE){ //file has hit max size
        printf("File too large!\n");
        return -1;
    }
    if(log_bn > 4){ //no more direct blocks, allocate in indirect block
        if(ip->i_addr[5] == 0) //indirect block still unallocated
        {
            ip->i_addr[5] = balloc();
        }
        unsigned int* b = malloc(BSIZE);
        bread(ip->i_addr[5], b);
        if(b[log_bn - 5] == 0){
            b[log_bn - 5] = balloc();
        }
        dsk_bn = b[log_bn - 5];
        bwrite(ip->i_addr[5], b);
        free(b); 
        return dsk_bn;
    }
    if(ip->i_addr[log_bn] == 0){ //allocate block if needed
        if(ip->i_number == 0 && log_bn > 0){ //root inode always owns block 0 as its first block
            ip->i_addr[log_bn] = balloc();
        } else if(ip->i_number != 0){
            ip->i_addr[log_bn] = balloc();
        }
    }
    return ip->i_addr[log_bn];
}
