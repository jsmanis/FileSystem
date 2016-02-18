OBJS 	= main.o mfs_functions.o utilities.o mfs_create.o mfs_import.o mfs_workwith.o mfs_mkdir.o mfs_cd.o mfs_pwd.o mfs_ls.o mfs_touch.o mfs_export.o
SOURCE	= main.c mfs_functions.c utilities.c mfs_create.c mfs_import.c mfs_workwith.c mfs_mkdir.c mfs_cd.c mfs_pwd.c mfs_ls.c mfs_touch.c mfs_export.c
OUT  	= mfs
CC	= gcc
FLAGS   = -g -c

all: $(OBJS)
	$(CC) -g -Wall $(OBJS) -o $(OUT)

main.o: main.c
	$(CC) $(FLAGS) main.c

mfs_functions.o: mfs_functions.c
	$(CC) $(FLAGS) mfs_functions.c

mfs_import.o: mfs_import.c
		$(CC) $(FLAGS) mfs_import.c

mfs_create.o: mfs_create.c
		$(CC) $(FLAGS) mfs_create.c

mfs_workwith.o: mfs_workwith.c
		$(CC) $(FLAGS) mfs_workwith.c

mfs_mkdir.o: mfs_mkdir.c
		$(CC) $(FLAGS) mfs_mkdir.c

mfs_cd.o: mfs_cd.c
		$(CC) $(FLAGS) mfs_cd.c

mfs_pwd.o: mfs_pwd.c
		$(CC) $(FLAGS) mfs_pwd.c

mfs_ls.o: mfs_ls.c
		$(CC) $(FLAGS) mfs_ls.c

mfs_touch.o: mfs_touch.c
		$(CC) $(FLAGS) mfs_touch.c

mfs_export.o: mfs_export.c
	$(CC) $(FLAGS) mfs_export.c

utilities.o: utilities.c
	$(CC) $(FLAGS) utilities.c
# clean object files of project
clean:
	rm -f $(OBJS) $(OUT)
