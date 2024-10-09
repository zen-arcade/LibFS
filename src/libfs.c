#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/libfs.h"
#include "../include/sys.h"
#include "../include/inode.h"
#include "../include/block.h"
#include "../include/diskio.h"

struct filsys* sb; 
unsigned int block_count = 0;
unsigned int inode_count = 0;
long int dinode_offset = 0;
long int bitmap_offset = 0;
long int blocks_offset = 0;
struct inode* root;
struct user current;
struct inode inode[NINODE];
FILE* dsk_libfs;

int close_libfs(){ 
    for(struct inode* ip = &inode[0]; ip < &inode[NINODE]; ip++){
        if(ip->i_mode != 0 ){
            iput(ip);
        }
    }
    for(int i = 0; i < NFILE; i++){
        if(current.u_ofile[i]->f_flag != 0){
            lfs_close(i);
        }
        free(current.u_ofile[i]);
    }
    fseek(dsk_libfs, 0, SEEK_SET);
    fwrite(sb, sizeof(struct filsys), 1, dsk_libfs);
    free(sb);
    fclose(dsk_libfs);
    

    return 0;
}

int init_libfs(){
    dsk_libfs = fopen("dsk-libfs.bin", "r+");
    if((dsk_libfs) == NULL){ //partition doesn't exist
        printf("Partition file not found!\n");
        return 1;
    }
    sb = malloc(sizeof(struct filsys));
    fread(sb, sizeof(struct filsys), 1, dsk_libfs);
    block_count = sb->s_fsize;
    inode_count = sb->s_isize;
    bitmap_offset = sizeof(struct filsys);
    dinode_offset = sizeof(struct filsys) + block_count;
    blocks_offset = sizeof(struct filsys) + block_count + inode_count*sizeof(struct dinode);
    current.u_cdir = 0;
    for(int i = 0; i < NFILE; i++){
        current.u_ofile[i] = malloc(sizeof(struct file));
    }

    return 0;
}

// int main(){
//     //to use as boilerplate for tests
//     if(init_libfs()){
//         
//         return 1;
//     }

//     close_libfs();
//     return 0;
// }
