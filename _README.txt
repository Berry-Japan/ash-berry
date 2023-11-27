export CVSROOT=:pserver:anoncvs@anoncvs.tw.FreeBSD.org:/home/ncvs
cvs login
cvs co src/bin/sh
cvs checkout src/bin/sh

http://www.in-ulm.de/~mascheck/various/ash/

CC="diet gcc" CFLAGS=-Os ./configure --with-libedit
