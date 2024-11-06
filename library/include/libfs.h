#ifndef LIBFS_H
#define LIBFS_H

/*
    Projet M1 SAR 2023/2024
    LibFS - par Ilyes Mortadha KADA BENABDALLAH
    Encadré par Pierre SENS
*/

#include <stdint.h>
#include <stdio.h>

#define BSIZE 512
#define NADDR 6

#define IFNORM 1 //normal file
#define IFDIR 2 //directory
#define IREAD 4 //read mode
#define IWRITE 8 //write mode
#define O_RDONLY 4
#define O_WRONLY 8
#define O_RDWR 12

#define DIRSIZ 12
#define NIFREE 100

#define BREAD 1

#define MAX_SIZE BSIZE*(NADDR-1)+BSIZE*128
#define MAX_BLKS NADDR + 127

extern struct filsys* sb;
extern unsigned int block_count;
extern unsigned int inode_count;
extern long int dinode_offset;
extern long int bitmap_offset;
extern long int blocks_offset;
extern FILE* dsk_libfs;

int init_libfs();
int close_libfs();

//disk inode
struct dinode {
    unsigned int di_mode; //file permissions
    unsigned int di_size; //file size in bytes
    unsigned int di_addr[NADDR]; //file data blocks
};

//directory
struct direct {
    unsigned int d_ino; //inode number
    char d_name[DIRSIZ]; //file name
};

//super block
struct filsys {
    unsigned int s_fsize; //partition size in blocks
    unsigned int s_isize; //number of inodes
    unsigned int s_ninode; //number of free inodes kept in super block
    unsigned int s_inode[NIFREE]; //free inodes kept in super block
};

//open files
struct file {
    char f_flag; //mode
    struct inode* f_inode; //pointer to inode
};

//current process
#define NFILE 16
extern struct user {
    int u_cdir; //inode number of current directory
    struct file* u_ofile[NFILE]; //list of open files
} current;

//inodes in memory
#define NINODE 200
extern struct inode {
    int i_number; //inode number, correspond to disk inode index
    unsigned int i_mode; //file permissions
    unsigned int i_size; //file size in bytes
    unsigned int i_addr[NADDR]; //file data blocks
    unsigned int i_count; //memory usage count 
} inode[NINODE];

#endif