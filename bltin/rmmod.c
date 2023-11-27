/*
 * cheap lsmod(1)-replacement --  author Klaus Knopper
 * needed for static ash on tight linux bootdisk
 */
#include <errno.h>
#include <fcntl.h>
#include "bltin.h"

static int syntax(void)
{
 error(
 "Usage: rmmod [-a | module]\n"
 "This is a builtin command.\n"
 );
 return -1;
}

int rmmodcmd(int argc, char **argv)
{
 int i;
 if(argc<2) return syntax();
 if(delete_module(strcmp(argv[i],"-a")?argv[1]:NULL)) { perror(argv[0]); return -1; }
 return 0;
}
