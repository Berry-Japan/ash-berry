/*
 * sleep(3);  --  author Klaus Knopper
 * needed for static ash on tight linux bootdisk
 */
#include <unistd.h>
#include "bltin.h"

static int syntax(void)
{
 error(
 "Usage: sleep seconds\n"
 );
 return -1;
}

int sleepcmd(int argc, char **argv)
{
 if(argc != 2) return syntax();
 /* Do the umount */
 if(sleep(atoi(argv[1]))!=0)
  { 
   perror(argv[0]);
   return -1;
  }
 return 0;
}
