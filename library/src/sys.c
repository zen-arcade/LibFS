#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/sys.h"
#include "../include/inode.h"
#include "../include/block.h"
#include "../include/diskio.h"

int lfs_mkdir(const char* pathname, int mode){
    struct inode* ip = ialloc();
    char* path = strdup(pathname);
    struct inode* parent = namei(path, 1);
    if(parent == NULL){
        ifree(ip->i_number);
        printf("mkdir: Directory with that name already exists.\n");
        return 1;
    }
    if(ip == NULL || (parent->i_mode & IWRITE) != IWRITE){
        printf("mkdir: Could not make directory.\n");
        return 1;
    }
    ip->i_mode = mode | IFDIR;
    ip->i_size = 0;
    char* tmp = strsep(&path, "/");
    char* name = malloc(DIRSIZ);
    int added = 0;
    //get last component of path
    while(tmp != NULL){
        name = tmp;
        tmp = strsep(&path, "/");
    }
    //look for empty directory entry and add name to parent
    struct direct* dir_entries = malloc(BSIZE);
    for(int i = 0; i <= parent->i_size/BSIZE; i++){
        if(added == 1) break;
        bread(bmap(parent, i, BREAD), dir_entries);
        for(struct direct* d = &dir_entries[0]; d < &dir_entries[32]; d++){
            if(d->d_ino == 0){
                memcpy(d->d_name, name, DIRSIZ);
                d->d_ino = ip->i_number;
                parent->i_size += sizeof(struct direct);
                added = 1;
                bwrite(bmap(parent, i, BREAD), dir_entries);
                break;
            }
        }
        bmap(parent, 0, 0); //allocate new block if needed
    }
    //initialize empty directory entries
    bmap(ip, 0, 0); //allocate 1st block
    bread(bmap(ip, 0, BREAD), dir_entries); 
    for(struct direct* d = &dir_entries[0]; d < &dir_entries[32]; d++){
        d->d_ino = 0;
    }
    bwrite(bmap(ip, 0, 0), dir_entries);
    free(dir_entries);
    iput(ip);
    iput(parent);
    return 0;
}

int lfs_chdir(const char* path){
    char* name = strdup(path);
    struct inode* ip = namei(name, 0);
    if(ip == NULL){
        printf("chdir: Directory not found.\n");
        return 1;
    }
    if((ip->i_mode & IFDIR) != IFDIR){
        printf("chdir: Not a directory.\n");
        return 1;
    }
    current.u_cdir = ip->i_number;
    iput(ip);
    return 0;
}

int lfs_unlink(const char* pathname){
    char* name = strdup(pathname);
    struct inode* parent = namei(name, 2);
    if(parent == NULL){
        printf("unlink: No such file or directory.\n");
        return 1;
    }
    struct inode* ip = namei(name, 0);
    struct direct* dir_entries = malloc(BSIZE);
    for(int i = 0; i <= parent->i_size/BSIZE; i++){
        bread(bmap(parent, i, BREAD), dir_entries);
        for(struct direct* d = &dir_entries[0]; d < &dir_entries[32]; d++){
            if(ip->i_number == d->d_ino){
                d->d_ino = 0;
                memset(d->d_name, 0, 12);
                ifree(ip->i_number);
                bwrite(bmap(parent, i, BREAD), dir_entries);
                return 0;
            }
        }
    }    
}

int lfs_creat(const char* pathname, int mode){
    struct inode* ip = ialloc();
    char* path = strdup(pathname);
    struct inode* parent = namei(path, 1);
    if(parent == NULL){
        iput(ip);
        if(ip != NULL) ifree(ip->i_number);
        printf("creat: File with that name already exists.\n");
        return 1;
    }
    if(ip == NULL || (parent->i_mode & IWRITE) != IWRITE){
        printf("creat: Could not create file.\n");
        return 1;
    }
    ip->i_mode = mode | IFNORM;
    ip->i_size = 0;
    char* tmp = strsep(&path, "/");
    char* name = malloc(DIRSIZ);
    int added = 0;
    //get last component of path
    while(tmp != NULL){
        name = tmp;
        tmp = strsep(&path, "/");
    }
    //look for empty directory entry and add name to parent
    struct direct* dir_entries = malloc(BSIZE);
    for(int i = 0; i <= parent->i_size/BSIZE; i++){
        if(added == 1) break;
        bread(bmap(parent, i, BREAD), dir_entries);
        for(struct direct* d = &dir_entries[0]; d < &dir_entries[32]; d++){
            if(d->d_ino == 0){
                memcpy(d->d_name, name, DIRSIZ);
                d->d_ino = ip->i_number;
                parent->i_size += sizeof(struct direct);
                added = 1;
                bwrite(bmap(parent, i, BREAD), dir_entries);
                break;
            }
        }
        bmap(parent, 0, 0); //allocate new block if needed
    }
    free(dir_entries);
    iput(ip);
    iput(parent);
    return 0;
}

int lfs_open(const char* pathname, int flags){
    char* name = strdup(pathname);
    struct inode* ip = namei(name, 0);
    if(flags == 0){
        iput(ip);
        printf("open: Must specify mode.\n");
        return -1;
    }
    if(ip == NULL){
        printf("open: File not found.\n");
        return -1;
    }
    if((ip->i_mode & IFDIR) == IFDIR){
        iput(ip);
        printf("open: Not a file.\n");
        return -1;
    }
    if((ip->i_mode & flags) != flags){
        iput(ip);
        if((ip->i_mode & IREAD) == IREAD){
            printf("open: Cannot open file for reading.\n");
        }
        if((ip->i_mode & IWRITE) == IWRITE){
            printf("open: Cannot open file for writing.\n");
        }
        return -1;
    }
    struct file* ofile = malloc(sizeof(struct file));
    ofile->f_flag = flags;
    ofile->f_inode = ip;
    int i = 0;
    for(int i = 0; i < NFILE; i++){
        if(current.u_ofile[i]->f_flag == 0){
            current.u_ofile[i]->f_flag = ofile->f_flag;
            current.u_ofile[i]->f_inode = ofile->f_inode;
            free(ofile);
            return i;
        }
        i++;
    }
    printf("open: Too many open files.\n");
    return -1;
}

int lfs_close(int fd){
    if(fd < 0 || current.u_ofile[fd]->f_flag == 0){
        printf("close: Invalid file desciptor.\n");
        return 1;
    }
    current.u_ofile[fd]->f_flag = 0;
    iput(current.u_ofile[fd]->f_inode);
    return 0;
}

int lfs_read(int fd, void* buf, int count){
    if(fd < 0){
        printf("read: Invalid file descriptor.\n");
        return -1;
    }
    if((current.u_ofile[fd]->f_flag & IREAD) != IREAD){
        printf("read: File not open for reading.\n");
        return -1;
    }
    struct inode* ip = current.u_ofile[fd]->f_inode;
    if(ip->i_size == 0){
        printf("read: File is empty.\n");
        return -1;
    }
    uint8_t* data = malloc(BSIZE);
    uint8_t* b_buf = (uint8_t*) buf; 
    int bytes_read = 0;
    for(int i = 0; i <= ip->i_size/BSIZE; i++){
        bread(bmap(ip, i, BREAD), data);
        for(uint8_t* b = &data[0]; b < &data[BSIZE]; b++){
            if(bytes_read == ip->i_size || bytes_read == count){
                free(data);
                return bytes_read;
            }
            *b_buf++ = *b;
            bytes_read++;
        }
    }
}

//fix to stop writing past buf
int lfs_write(int fd, void* buf, int count){
    if(fd < 0){
        printf("read: Invalid file descriptor.\n");
        return -1;
    }
    if((current.u_ofile[fd]->f_flag & IWRITE) != IWRITE){
        printf("read: File not open for writing.\n");
        return -1;
    }
    struct inode* ip = current.u_ofile[fd]->f_inode;
    int written = 0;
    int blkno = bmap(ip, 0, 0);
    int last_blk = ip->i_size / BSIZE;
    int offset = ip->i_size % BSIZE;
    uint8_t* data = malloc(BSIZE);
    uint8_t* b_buf = (uint8_t*) buf;
    for(int i = last_blk; i < MAX_BLKS; i++){
        bread(bmap(ip, i, BREAD), data);
        for(uint8_t* b = &data[offset]; b < &data[BSIZE]; b++){
            if(written == count){
                bwrite(bmap(ip, i, BREAD), data);
                free(data);
                return written;
            }
            *b = *b_buf++;
            written++;
            ip->i_size++;
            bmap(ip, 0, 0);
            if(((written+offset) % BSIZE) == 0){
                bwrite(bmap(ip, i, BREAD), data);
                offset = 0;
            }
        }
    }
}