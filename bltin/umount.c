/*
 * umount(1); version 2-like  --  author Klaus Knopper
 * needed for static ash on tight linux bootdisk
 */
#include <sys/mount.h>
#include "bltin.h"

static int syntax(void)
{
 error(
 "Usage: umount device\n"
 "This is a builtin command. /etc/fstab and /etc/mtab are NOT supported.\n"
 );
 return -1;
}

int umountcmd(int argc, char **argv)
{
 if(argc<1) return syntax();
 /* Do the umount */
 if(umount(argv[1])!=0)
  { 
   perror(argv[0]);
   return -1;
  }
 return 0;
}
