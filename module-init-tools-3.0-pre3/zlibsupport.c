/* Support for compressed modules.  Willy Tarreau <willy@meta-x.org>
 * did the support for modutils, Andrey Borzenkov <arvidjaar@mail.ru>
 * ported it to module-init-tools, and I said it was too ugly to live
 * and rewrote it 8).
 *
 * (C) 2003 Rusty Russell, IBM Corporation.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "zlibsupport.h"
#include "testing.h"

#ifdef CONFIG_USE_ZLIB
#include <zlib.h>

/* gzopen handles uncompressed files transparently. */
void *grab_file(const char *filename, unsigned long *size)
{
	unsigned int max = 16384;
	int ret;
	gzFile fd;
	void *buffer = malloc(max);

	fd = gzopen(filename, "rb");
	if (!fd)
		return NULL;

	*size = 0;
	while ((ret = gzread(fd, buffer + *size, max - *size)) > 0) {
		*size += ret;
		if (*size == max)
			buffer = realloc(buffer, max *= 2);
	}
	if (ret < 0) {
		free(buffer);
		buffer = NULL;
	}
	gzclose(fd);
	return buffer;
}

void release_file(void *data, unsigned long size)
{
	free(data);
}
#else /* ... !CONFIG_USE_ZLIB */

void *grab_file(const char *filename, unsigned long *size)
{
	int fd;
	struct stat st;
	void *map;

	fd = open(filename, O_RDONLY, 0);
	if (fd < 0)
		return NULL;

	fstat(fd, &st);
	*size = st.st_size;
	map = mmap(0, *size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
		map = NULL;

	close(fd);
	return map;
}

void release_file(void *data, unsigned long size)
{
	munmap(data, size);
}
#endif
