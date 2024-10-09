#ifndef BLOCK_H
#define BLOCK_H
#include "libfs.h"

int balloc();
void bfree(int bn);
int bmap(struct inode* ip, int bn, int flag);

#endif