//---------------------------------------------------------
//	reread partition table
//
//		(C)2005 NAKADA
//---------------------------------------------------------

#include <fcntl.h>	/* O_RDWR */
#include <linux/fs.h>	/* BLKGETSIZE */


/* tell the kernel to reread the partition tables */
int reread_ioctl(int fd)
{
	if (ioctl(fd, BLKRRPART)) {
		perror("BLKRRPART");
		return -1;
	}
	return 0;
}


//---------------------------------------------------------
//	main
//---------------------------------------------------------

int rescan_main(int argc, const char **argv)
{
	int r;
	int fd;

	fd=open(argv[1], O_RDONLY);
	r=reread_ioctl(fd);
	close(fd);

	return;
}
