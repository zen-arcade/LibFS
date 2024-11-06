# LibFS 

LibFS is a simplified Unix filesystem emulator based on the Unix source code level six with help from the 6th edition of John Lions' book.

TODO add inode structures etc

### Usage
Run mklibfs to create the dsk-libfs file, simulating a standard Unix partition (without boot block).
Must specify number of blocks and number of inodes.

To use in user applications, include the "sys.h" and "libfs.h" header files.
Compile with needed .c files : block.c diskio.c inode.c libfs.c sys.c

Initialize with init_libfs() and call close_libfs() when done.





Projet SAR - Sorbonne Université
Encadré par Pierre SENS @ LIP6
