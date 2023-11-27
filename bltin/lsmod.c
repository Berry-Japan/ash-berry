/*
 * cheap lsmod(1)-replacement --  author Klaus Knopper
 * needed for static ash on tight linux bootdisk
 */
#include <errno.h>
#include <fcntl.h>
#include "bltin.h"

int lsmodcmd(int argc, char **argv)
{
 int f;
 const char *mfile="/proc/modules";
 char buffer[1024];
 int rc=0;
 if((f=open(mfile,O_RDONLY))>=0)
  {
   size_t r;
   while((r=read(f,buffer,1024))>0) write(1,buffer,r);
   close(f);
  }
 else { rc=errno; error("Cannot open %s.",mfile); }
 return rc;
}
