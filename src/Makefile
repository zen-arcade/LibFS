DEP :=  ../src/inode.c ../src/libfs.c ../src/sys.c ../src/diskio.c ../src/block.c

all: mkdir_test chdir_test recursion_test rec_chdir_test creat_test single_wr_test single_rd_test write_test read_test unlink_test present_test

mkdir_test: $(DEP) mkdir_test.c
	gcc $(DEP) mkdir_test.c -g -o mkdir_test

chdir_test: $(DEP) chdir_test.c
	gcc $(DEP) chdir_test.c -g -o chdir_test

recursion_test: $(DEP) recursion_test.c
	gcc $(DEP) recursion_test.c -g -o recursion_test

rec_chdir_test: $(DEP) rec_chdir_test.c
	gcc $(DEP) rec_chdir_test.c -g -o rec_chdir_test

creat_test: $(DEP) creat_test.c
	gcc $(DEP) creat_test.c -g -o creat_test

single_wr_test: $(DEP) single_wr_test.c
	gcc $(DEP) single_wr_test.c -g -o single_wr_test

single_rd_test: $(DEP) single_rd_test.c
	gcc $(DEP) single_rd_test.c -g -o single_rd_test

write_test: $(DEP) write_test.c
	gcc $(DEP) write_test.c -g -o write_test

read_test: $(DEP) read_test.c
	gcc $(DEP) read_test.c -g -o read_test

unlink_test: $(DEP) unlink_test.c
	gcc $(DEP) unlink_test.c -g -o unlink_test

present_test: $(DEP) present_test.c
	gcc $(DEP) present_test.c -g -o present_test