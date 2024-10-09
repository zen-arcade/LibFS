#ifndef SYS_H
#define SYS_H

int lfs_mkdir(const char* pathname, int mode);
int lfs_chdir(const char* path);
int lfs_unlink(const char* pathname);
int lfs_creat(const char* pathname, int mode);
int lfs_open(const char* pathname, int flags);
int lfs_read(int fd, void* buf, int count);
int lfs_write(int fd, void* buf, int count);
int lfs_close(int fd);


#endif