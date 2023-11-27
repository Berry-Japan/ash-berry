PROG=	sh
SRCS=	builtins.c cd.c dirent.c bltin/echo.c error.c eval.c exec.c expand.c \
	input.c jobs.c mail.c main.c memalloc.c miscbltin.c \
	mystring.c nodes.c options.c parser.c redir.c show.c \
	syntax.c trap.c output.c var.c bltin/test.c \
	bltin/mount.c bltin/umount.c bltin/cat.c bltin/lsmod.c bltin/rmmod.c \
	bltin/losetup.c

OBJ1 =	init.o
OBJ2 =	builtins.o cd.o dirent.o bltin/echo.o error.o eval.o exec.o expand.o \
	input.o jobs.o mail.o main.o memalloc.o miscbltin.o \
	mystring.o nodes.o options.o parser.o redir.o show.o \
	syntax.o trap.o output.o var.o bltin/test.o \
	bltin/mount.o bltin/umount.o bltin/cat.o bltin/lsmod.o bltin/rmmod.o \
	bltin/losetup.o

MOD_OBJ = ./modutils-2.4.18/insmod/insmod.o \
	./modutils-2.4.18/obj/libobj.a ./modutils-2.4.18/util/libutil.a

OBJS =	$(OBJ1) $(OBJ2)

#CFLAGS = $(RPM_OPT_FLAGS) -DSHELL -I/usr/include/linux -I./modutils-2.4.15/include -I. -D__BIT_TYPES_DEFINED__
CFLAGS = $(RPM_OPT_FLAGS) -DSHELL -I. -D__BIT_TYPES_DEFINED__
LDFLAGS += -s

CLEANFILES =\
	builtins.c builtins.h init.c mkinit mknodes mksyntax \
	nodes.c nodes.h syntax.c syntax.h token.def

all:	$(OBJS) $(MOD_LIB) $(MOD_OBJ)
	$(CC) $(STATIC) -o $(PROG) $(OBJS) $(MOD_OBJ) $(LDFLAGS)

install: all
	install sh /bin/ash
	install -m 644 sh.1 /usr/man/man1/ash.1

parser.o: token.def
token.def: mktokens
	sh ./mktokens

builtins.h builtins.c: mkbuiltins
	sh ./mkbuiltins

init.c: mkinit $(SRCS)
	./mkinit '${CC} -c $(CFLAGS) init.c' $(SRCS)
	touch init.c

mkinit: mkinit.c
	$(CC) $(CFLAGS) $(LDFLAGS) mkinit.c -o $@ $(LDADD)

nodes.c nodes.h: mknodes nodetypes nodes.c.pat
	./mknodes nodetypes nodes.c.pat

mknodes: mknodes.c
	$(CC) $(CFLAGS) $(LDFLAGS) mknodes.c -o $@ $(LDADD)

syntax.c syntax.h: mksyntax
	./mksyntax

mksyntax: mksyntax.c parser.h
	$(CC) $(CFLAGS) $(LDFLAGS) mksyntax.c -o $@ $(LDADD)

clean:
	rm -f core $(CLEANFILES) $(PROG) $(OBJS)

.c.o:
	${CC} ${CFLAGS} -o $@ -c $<

