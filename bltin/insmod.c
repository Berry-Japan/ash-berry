//---------------------------------------------------------
//	insmod(1)-replacement
//	needed for static ash on tight linux bootdisk
//
//		(C)2005 NAKADA
//---------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <asm/unistd.h>

//#include "backwards_compat.c"

#define streq(a,b) (strcmp((a),(b)) == 0)

extern long init_module(void *, unsigned long, const char *);

//static void print_usage(const char *progname)
static int print_usage(const char *progname)
{
	fprintf(stderr, "Usage: %s filename [args]\n", progname);
	//exit(1);
	return 1;
}

/* We use error numbers in a loose translation... */
static const char *moderror(int err)
{
	switch (err) {
	case ENOEXEC:
		return "Invalid module format";
	case ENOENT:
		return "Unknown symbol in module";
	case ESRCH:
		return "Module has wrong symbol version";
	case EINVAL:
		return "Invalid parameters";
	default:
		return strerror(err);
	}
}

static void *grab_file(const char *filename, unsigned long *size)
{
	unsigned int max = 16384;
	int ret, fd;
	void *buffer = malloc(max);

	if (streq(filename, "-"))
		fd = dup(STDIN_FILENO);
	else
		fd = open(filename, O_RDONLY, 0);

	if (fd < 0)
		return NULL;

	*size = 0;
	while ((ret = read(fd, buffer + *size, max - *size)) > 0) {
		*size += ret;
		if (*size == max)
			buffer = realloc(buffer, max *= 2);
	}
	if (ret < 0) {
		free(buffer);
		buffer = NULL;
	}
	close(fd);
	return buffer;
}

int insmodcmd(int argc, char *argv[])
{
	unsigned int i;
	long int ret;
	unsigned long len;
	void *file;
	char *filename, *options = strdup("");
	char *progname = argv[0];

	/*if (strstr(argv[0], "insmod.static"))
		try_old_version("insmod.static", argv);
	else
		try_old_version("insmod", argv);*/

	/*if (argv[1] && (streq(argv[1], "--version") || streq(argv[1], "-V"))) {
		puts(PACKAGE " version " VERSION);
		//exit(0);
		return 0;
	}*/

	/* Ignore old options, for backwards compat. */
	while (argv[1] && (streq(argv[1], "-p")
			|| streq(argv[1], "-s")
			|| streq(argv[1], "-f"))) {
		argv++;
		argc--;
	}

	filename = argv[1];
	if (!filename) return print_usage(progname);

	/* Rest is options */
	for (i = 2; i < argc; i++) {
		options = realloc(options,
				  strlen(options) + 2 + strlen(argv[i]) + 2);
		/* Spaces handled by "" pairs, but no way of escaping
                   quotes */
		if (strchr(argv[i], ' '))
			strcat(options, "\"");
		strcat(options, argv[i]);
		if (strchr(argv[i], ' '))
			strcat(options, "\"");
		strcat(options, " ");
	}

	file = grab_file(filename, &len);
	if (!file) {
		fprintf(stderr, "insmod: can't read '%s': %s\n",
			filename, strerror(errno));
		//exit(1);
		return 1;
	}

	ret = init_module(file, len, options);
	if (ret != 0) {
		fprintf(stderr, "insmod: error inserting '%s': %li %s\n",
			filename, ret, moderror(errno));
		//exit(1);
		return 1;
	}
	//exit(0);
	return 0;
}
