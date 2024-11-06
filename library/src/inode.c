#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "../include/sys.h"
#include "../include/inode.h"
#include "../include/block.h"
#include "../include/diskio.h"

//load an inode into memory, or retrieve from memory inode table
struct inode* iget(int ino){
    //look in memory inode table if already present
    for(struct inode* p = &inode[0]; p < &inode[NINODE]; p++){
        if(p->i_number == ino && p->i_mode != 0){
            p->i_count++;
            return p;
        }
    }
    //if not present, get from disk and put in memory
    //position offset at disk inode table + dinode number
    fseek(dsk_libfs, dinode_offset + ino*sizeof(struct dinode), SEEK_SET);
    struct dinode* tmp_di = malloc(sizeof(struct dinode));
    fread(tmp_di, sizeof(struct dinode), 1, dsk_libfs);
    struct inode* ip = NULL;
    for(struct inode* p = &inode[0]; p < &inode[NINODE];p++){
        if(ip == NULL && p->i_count == 0){ //overwrite first unused inode in memory
            ip = p;
            break;
        }
    }
    if(ip == NULL){
        //inode table full
        return NULL;
    }
    ip->i_count++;
    ip->i_number = ino;
    ip->i_mode = tmp_di->di_mode;
    ip->i_size = tmp_di->di_size;
    memcpy(ip->i_addr, tmp_di->di_addr, NADDR*sizeof(unsigned int));
    free(tmp_di);
    return ip;
}

//decrement usage count of inode, if no longer used save to disk
//when i_count = 0, effectively free as iget will overwrite it
void iput(struct inode* ip){
    if(ip->i_count == 1){
        struct dinode* dip = malloc(sizeof(struct dinode));
        dip->di_mode = ip->i_mode;
        dip->di_size = ip->i_size;
        memcpy(dip->di_addr, ip->i_addr, NADDR*sizeof(unsigned int));
        fseek(dsk_libfs, dinode_offset + ip->i_number*sizeof(struct dinode), SEEK_SET);
        fwrite(dip, sizeof(struct dinode), 1, dsk_libfs);
        free(dip);
    }
    ip->i_count--;
}

//allocate a free inode from disk 
struct inode* ialloc(){
    //if super block empty, fill it up with free inodes from disk
    int free = 0;
    struct inode* ip = NULL;
    if(sb->s_ninode == 0){
        struct dinode* dip = malloc(sizeof(struct dinode));
        for(int ino = 0; ino < inode_count; ino++){
            fseek(dsk_libfs, dinode_offset + ino*sizeof(struct dinode), SEEK_SET);
            fread(dip, sizeof(struct dinode), 1, dsk_libfs);
            if(dip->di_mode == 0){
                for(int i = 0; i < NIFREE; i++){
                    if(sb->s_inode[i] == 0){
                        sb->s_inode[i] = ino;
                        free++;
                        break;
                    }
                }            
            }
        }
        if(free == 0){
            printf("Disk inode table is full.\n");
            return NULL;
        }
        sb->s_ninode = free;
    }
    //super block has free inodes, get the first one. root inode (0) is never free
    for(int i = 0; i < NIFREE; i++){
        if(sb->s_inode[i] != 0){
            ip = iget(sb->s_inode[i]);
            sb->s_ninode--;
            sb->s_inode[i] = 0;
            return ip;
        }
    }
    return ip;
}

//free an inode from disk and deallocate its blocks, then add to superblock free inodes
void ifree(int ino){
    struct inode* ip = iget(ino);
    if(ip == NULL) return;
    ip->i_mode = 0;
    int b;
    for(int i = 0; i <= ip->i_size/BSIZE;i++){ //free data blocks
        b = bmap(ip, i, BREAD);
        if(b == 0) continue;
        bfree(b); 
    }
    if(ip->i_addr[5] != 0){
        bfree(ip->i_addr[5]); //free indirect block
    }
    ip->i_size = 0;
    memset(ip->i_addr, 0, NADDR*sizeof(unsigned int));
    while(ip->i_count > 0){
        iput(ip);
    }
    for(int i = 0; i < NIFREE; i++){
        if(sb->s_inode[i] == 0){
            sb->s_inode[i] = ino;
            sb->s_ninode++;
            break;
        }
    }
}

//return pointer to inode with the specified name
//flag 0 = file exists, 1 = create file, 2 = delete file
struct inode* namei(char* name, int flag){
    struct inode* ip;
    struct direct* dir_entries = malloc(BSIZE);
    char* tmp = strdup(name);
    char* path = malloc(DIRSIZ);
    if(name[0] == '/'){ //start from root
        ip = iget(0);
        path = strsep(&tmp, "/"); //consume leading slash
    } else {
        ip = iget(current.u_cdir);
    }
        int found = 0;
        int last_dir = 0;
        //32 directory entries per block
        path = strsep(&tmp, "/");
        while(path != NULL && ip->i_mode & IFDIR|IREAD){
            found = 0;
            for(int i = 0; i <= ip->i_size/BSIZE; i++){
                bread(bmap(ip, i, BREAD), dir_entries);
                for(struct direct* d = &dir_entries[0]; d < &dir_entries[32]; d++){
                    if(strcmp(path, d->d_name) == 0){
                        found = 1;
                        last_dir = ip->i_number;
                        iput(ip);
                        ip = iget(d->d_ino);
                        break;
                    }
                }
                if(path == NULL) break;
            }
            path = strsep(&tmp, "/");
        }
    //if found = 0, last matched component is parent dir of file to be created (flag == 1), or not found (0 or 2)
    //else found = 1, file found to be returned (0) or deleted (2)
    if(found){
        if(flag == 1 || flag == 2){
            if(flag == 1) return NULL; //name conflict
            iput(ip);
            ip = iget(last_dir); //return parent dir to unlink()
        }
        return ip;
    } else if(!found){
        if(flag == 0 || flag == 2){ //not found
            return NULL;
        } 
        return ip;
    }
}