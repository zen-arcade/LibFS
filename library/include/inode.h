#ifndef INODE_H
#define INODE_H
#include "libfs.h"

struct inode* iget(int ino);
void iput(struct inode* ip);
struct inode* ialloc();
void ifree(int ino);
struct inode* namei(char* name, int flag);

#endif