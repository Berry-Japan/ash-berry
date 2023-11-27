/*
 * mount(1); version 2-like
 * needed for static ash on tight linux bootdisk
 */
#include <sys/mount.h>
#include <stdio.h>
#include <errno.h>
#include "bltin.h"

extern char *strtok(char *s, const char *delim);

static int syntax(void)
{
 error(
 "Usage: mount [-t filesystemtype] [-o options,...] device mountpoint\n"
 "This is a builtin command. /etc/fstab and /etc/mtab are NOT supported.\n"
 );
 return -1;
}

#ifndef MS_MGC_VAL
#define MS_MGC_VAL 0xc0ed0000
#endif
#ifndef MS_RDONLY
#define MS_RDONLY       1
#endif
#ifndef MS_NOSUID
#define MS_NOSUID	2
#endif
#ifndef MS_NODEV
#define MS_NODEV 	4
#endif
#ifndef MS_NOEXEC
#define MS_NOEXEC	8
#endif
#ifndef MS_SYNCHRONOUS
#define MS_SYNCHRONOUS	16
#endif
#ifndef MS_REMOUNT
#define MS_REMOUNT	32
#endif
#define MS_NOATIME	1024	/* Do not update access times. */
#define MS_NODIRATIME	2048	/* Do not update directory access times */

int mountcmd(int argc, char **argv)
{
 int i,rc;
 unsigned long fl=MS_MGC_VAL;
 char *dv=NULL,*mp=NULL,*fs=NULL;

 if(argc<2) /* List mounts, use procfs, simple "cat" */
  {
	char *cat_argv[2];
	cat_argv[0]=NULL;
	cat_argv[1]="/proc/mounts";
	return catcmd(2, cat_argv);
/* const char *pm="/proc/mounts";
   return catcmd(2,pm);*/
  }
 if(argc<3) return syntax();
 for(i=1;i<argc;i++)
  {
   if(!strcmp(argv[i],"-o"))
    {
     char *o; const char *d=",";
     ++i;
     if(i==argc) return syntax();
     o=strtok(argv[i],d);
     while(o!=NULL)
      {
	if (!strcmp(o, "ro"))			fl|=MS_RDONLY;
	else if (!strcmp(o, "rw"))		fl&=~MS_RDONLY;
	else if (!strcmp(o, "nosuid"))	fl|=MS_NOSUID;
	else if (!strcmp(o, "suid"))		fl&=~MS_NOSUID;
	else if (!strcmp(o, "nodev"))	fl|=MS_NODEV;
	else if (!strcmp(o, "dev"))		fl&=~MS_NODEV;
	else if (!strcmp(o, "noexec"))	fl|=MS_NOEXEC;
	else if (!strcmp(o, "exec"))		fl&=~MS_NOEXEC;
	else if (!strcmp(o, "sync"))		fl|=MS_SYNCHRONOUS;
	else if (!strcmp(o, "nosync"))	fl&=~MS_SYNCHRONOUS;
	else if (!strcmp(o, "remount"))	fl|=MS_REMOUNT;
	else if (!strcmp(o, "noatime"))	fl|=MS_NOATIME;
	else if (!strcmp(o, "nodiratime"))	fl|=MS_NODIRATIME;
	else error("Unknown option '%s' ignored.", o);
	o=strtok(NULL,d);
      }
    }
   else if(!strcmp(argv[i],"-t"))
    {
     ++i;
     if(i==argc) return syntax();
     fs=argv[i];
    }
   else if(dv==NULL) dv=argv[i];
   else if(mp==NULL) mp=argv[i];
   else return syntax();
  }
 /* Do the mount */
 rc=mount(dv,mp,fs,fl,NULL);
 if(rc==EACCES) /* Read-only filesystem */
   rc=mount(dv,mp,fs,fl|MS_RDONLY,NULL);
 if(rc!=0)
  {
   perror(argv[0]);
   return -1;
  }
 return 0;
}
