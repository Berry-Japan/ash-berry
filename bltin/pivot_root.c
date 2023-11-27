//---------------------------------------------------------
//	pivot_root - change the root device
//
//		(C)2003-2005,2010 NAKADA
//---------------------------------------------------------

#include <stdio.h>
#include <errno.h>

extern int pivot_root(const char * new_root,const char * put_old);
/*#ifdef __ia64__
	#include <sys/syscall.h>
	#define pivot_root(new_root,put_old) syscall(SYS_pivot_root,new_root,put_old)
#else
	#include <linux/unistd.h>
	static _syscall2(int,pivot_root,const char *,new_root,const char *,put_old)
#endif*/


//---------------------------------------------------------
//	main
//---------------------------------------------------------

int pivot_root_main(int argc, const char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s new_root put_old\n", argv[0]);
		return 1;
	}
	if (pivot_root(argv[1], argv[2]) < 0) {
		perror("pivot_root");
		return 1;
	}

	return 0;
}
