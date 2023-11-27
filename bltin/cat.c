/*
 * cheap cat(1)-replacement --  author Klaus Knopper
 * needed for static ash on tight linux bootdisk
 */
#include <errno.h>
#include <fcntl.h>
#include "bltin.h"

static void dump_file(int fd)
{
 size_t r,i;
 char buffer[1024];
 while((r=read(fd,buffer,1024))>0) { for(i=0;i<r;i++) putchar(buffer[i]); }
}

int catcmd(int argc, char **argv)
{
 int rc=0;
 if(argc<2) dump_file(0);
 else
  {
   int i;
   for(i=1;i<argc;i++)
    {
     int f;
     if((f=open(argv[i],O_RDONLY))>=0)
      {
       dump_file(f);
       close(f);
      }
     else { rc=errno; error("No file %s.",argv[i]); }
    }
  }
 return rc;
}
