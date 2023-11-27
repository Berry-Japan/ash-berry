/* Extract module info: useful for both the curious and for scripts. */
#define _GNU_SOURCE /* asprintf rocks */
#include <elf.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include "zlibsupport.h"
#include "backwards_compat.c"

#define streq(a,b) (strcmp((a),(b)) == 0)

#ifndef MODULE_DIR
#define MODULE_DIR "/lib/modules"
#endif

static int elf_endian;
static int my_endian;

static inline void __endian(const void *src, void *dest, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++)
		((unsigned char*)dest)[i] = ((unsigned char*)src)[size - i-1];
}

#define TO_NATIVE(x)							  \
({									  \
	typeof(x) __x;							  \
	if (elf_endian != my_endian) __endian(&(x), &(__x), sizeof(__x)); \
	else __x = x;							  \
	__x;								  \
})

static void *get_section32(void *file, unsigned long *size, const char *name)
{
	Elf32_Ehdr *hdr = file;
	Elf32_Shdr *sechdrs = file + TO_NATIVE(hdr->e_shoff);
	const char *secnames;
	unsigned int i;

	secnames = file
		+ TO_NATIVE(sechdrs[TO_NATIVE(hdr->e_shstrndx)].sh_offset);
	for (i = 1; i < TO_NATIVE(hdr->e_shnum); i++)
		if (streq(secnames + TO_NATIVE(sechdrs[i].sh_name), name)) {
			*size = TO_NATIVE(sechdrs[i].sh_size);
			return file + TO_NATIVE(sechdrs[i].sh_offset);
		}
	return NULL;
}

static void *get_section64(void *file, unsigned long *size, const char *name)
{
	Elf64_Ehdr *hdr = file;
	Elf64_Shdr *sechdrs = file + TO_NATIVE(hdr->e_shoff);
	const char *secnames;
	unsigned int i;

	secnames = file
		+ TO_NATIVE(sechdrs[TO_NATIVE(hdr->e_shstrndx)].sh_offset);
	for (i = 1; i < TO_NATIVE(hdr->e_shnum); i++)
		if (streq(secnames + TO_NATIVE(sechdrs[i].sh_name), name)) {
			*size = TO_NATIVE(sechdrs[i].sh_size);
			return file + TO_NATIVE(sechdrs[i].sh_offset);
		}
	return NULL;
}

static int elf_ident(void *mod, unsigned long size)
{
	/* "\177ELF" <byte> where byte = 001 for 32-bit, 002 for 64 */
	char *ident = mod;

	if (size < EI_CLASS || memcmp(mod, ELFMAG, SELFMAG) != 0)
		return ELFCLASSNONE;
	elf_endian = ident[EI_DATA];
	return ident[EI_CLASS];
}

static void *get_section(void *file, unsigned long filesize,
			 unsigned long *size, const char *name)
{
	switch (elf_ident(file, filesize)) {
	case ELFCLASS32:
		return get_section32(file, size, name);
	case ELFCLASS64:
		return get_section64(file, size, name);
	default:
		return NULL;
	}
}

static const char *next_string(const char *string, unsigned long *secsize)
{
	/* Skip non-zero chars */
	while (string[0]) {
		string++;
		if ((*secsize)-- <= 1)
			return NULL;
	}

	/* Skip any zero padding. */
	while (!string[0]) {
		string++;
		if ((*secsize)-- <= 1)
			return NULL;
	}
	return string;
}

static void print_tag(const char *tag, const char *info, unsigned long size,
		      char sep)
{
	unsigned int taglen = strlen(tag);

	for (; info; info = next_string(info, &size))
		if (strncmp(info, tag, taglen) == 0 && info[taglen] == '=')
			printf("%s%c", info + taglen + 1, sep);
}

static void print_all(const char *info, unsigned long size, char sep)
{
	for (; info; info = next_string(info, &size)) {
		char *eq;
		if (!sep) {
			printf("%s%c", info, sep);
			continue;
		}

		eq = strchr(info, '=');
		/* Warn if no '=' maybe? */
		if (eq) {
			char tag[eq - info + 2];
			strncpy(tag, info, eq - info);
			tag[eq-info] = ':';
			tag[eq-info+1] = '\0';
			printf("%-16s%s%c", tag, eq+1, sep);
		}
	}
}

static struct option options[] =
{
	{"author", 0, 0, 'a'},
	{"description", 0, 0, 'd'},
	{"license", 0, 0, 'l'},
	{"parameters", 0, 0, 'p'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'h'},
	{"null", 0, 0, '0'},
	{"field", 0, 0, 'F'},
	{0, 0, 0, 0}
};

/* - and _ are equivalent, and expect .ko or .ko.gz suffix. */
static int name_matches(const char *basename, const char *modname)
{
	unsigned int i;

	for (i = 0; modname[i]; i++) {
		if (modname[i] == basename[i])
			continue;
		if (modname[i] == '_' && basename[i] == '-')
			continue;
		if (modname[i] == '-' && basename[i] == '_')
			continue;
		return 0;
	}
	if (streq(basename+i, ".ko") || streq(basename+i, ".ko.gz"))
		return 1;
	return 0;
}

static void *find_module(const char *dirname, const char *modname,
			 unsigned long *size)
{
	DIR *dir;
	struct dirent *dirent;

	dir = opendir(dirname);
	if (!dir)
		return NULL;

	while ((dirent = readdir(dir)) != NULL) {
		char fullpath[strlen(dirname) + 1 + strlen(dirent->d_name)+1];

		sprintf(fullpath, "%s/%s", dirname, dirent->d_name);

		if (name_matches(dirent->d_name, modname))
			return grab_file(fullpath, size);

		if (!streq(dirent->d_name,".") && !streq(dirent->d_name,"..")){
			struct stat st;
			if (lstat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
				void *data = find_module(fullpath, modname,
							 size);
				if (data) {
					closedir(dir);
					return data;
				}
			}
		}
	}
	closedir(dir);
	return NULL;
}


static void *grab_module(const char *name, unsigned long *size)
{
	void *data;
	struct utsname buf;
	char *dirname;

	data = grab_file(name, size);
	if (data)
		return data;
	if (errno != ENOENT) {
		fprintf(stderr, "modinfo: could not open %s: %s\n",
			name, strerror(errno));
		return NULL;
	}

	/* Search for it. */
	uname(&buf);
	asprintf(&dirname, "%s/%s", MODULE_DIR, buf.release);
	data = find_module(dirname, name, size);
	if (!data)
		fprintf(stderr, "modinfo: could not find module %s\n", name);
	free(dirname);
	return data;
}

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-0][-F field] module...\n"
		" Prints out the information about one or more module(s).\n"
		" If a fieldname is given, just print out that field (or nothing if not found).\n"
		" Otherwise, print all information out in a readable form\n"
		" If -0 is given, separate with nul, not newline.\n",
		name);
}

int main(int argc, char *argv[])
{
	union { short s; char c[2]; } endian_test;
	const char *field = NULL;
	char sep = '\n';
	unsigned long infosize;
	int opt, ret = 0;

	if (!getenv("NEW_MODINFO"))
		try_old_version("modinfo", argv);

	endian_test.s = 1;
	if (endian_test.c[1] == 1) my_endian = ELFDATA2MSB;
	else if (endian_test.c[0] == 1) my_endian = ELFDATA2LSB;
	else
		abort();

	while ((opt = getopt_long(argc,argv,"adlpVhn0F:",options,NULL)) >= 0){
		switch (opt) {
		case 'a': field = "author"; break;
		case 'd': field = "description"; break;
		case 'l': field = "license"; break;
		case 'p': field = "parm"; break;
		case 'V': printf(PACKAGE " version " VERSION "\n"); exit(0);
		case 'F': field = optarg; break;
		case '0': sep = '\0'; break;
		default:
			usage(argv[0]); exit(0);
		}
	}
	if (argc < optind + 1)
		usage(argv[0]);

	for (opt = optind; opt < argc; opt++) {
		void *info, *mod;
		unsigned long modulesize;

		mod = grab_module(argv[opt], &modulesize);
		if (!mod) {
			ret = 1;
			continue;
		}

		info = get_section(mod, modulesize, &infosize, ".modinfo");
		if (!info)
			continue;
		if (field)
			print_tag(field, info, infosize, sep);
		else
			print_all(info, infosize, sep);
	}
	return ret;
}
