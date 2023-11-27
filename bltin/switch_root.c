//---------------------------------------------------------
//	switch_root
//
//		(C)2010 Yuichiro Nakada
//---------------------------------------------------------

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/fs.h>
//#include <sys/vfs.h>
//#include <sys/mount.h>
//#include <sys/types.h>


#define DOT_OR_DOTDOT(s) ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))

// Die with an error message if we can't malloc() enough space and do an
// sprintf() into that space.
char *xasprintf(const char *format, ...)
{
	va_list p;
	int r;
	char *string_ptr;

	// GNU extension
	va_start(p, format);
	r = vasprintf(&string_ptr, format, p);
	va_end(p);

	if (r < 0) return 0;
	//	bb_error_msg_and_die(bb_msg_memory_exhausted);
	return string_ptr;
}

char *last_char_is(const char *s, int c)
{
	if (s && *s) {
		size_t sz = strlen(s) - 1;
		s += sz;
		if ( (unsigned char)*s == c) return (char*)s;
	}
	return NULL;
}

char *concat_path_file(const char *path, const char *filename)
{
	char *lc;

	if (!path) path = "";
	lc = last_char_is(path, '/');
	while (*filename == '/') filename++;
	return xasprintf("%s%s%s", path, (lc==NULL ? "/" : ""), filename);
}

// Recursively delete contents of rootfs
static void delete_contents(const char *directory, dev_t rootdev)
{
	DIR *dir;
	struct dirent *d;
	struct stat st;

	// Don't descend into other filesystems
	if (lstat(directory, &st) || st.st_dev != rootdev)
		return;

	// Recursively delete the contents of directories
	if (S_ISDIR(st.st_mode)) {
		dir = opendir(directory);
		if (dir) {
			while ((d = readdir(dir))) {
				char *newdir = d->d_name;

				// Skip . and ..
				if (DOT_OR_DOTDOT(newdir))
					continue;

				// Recurse to delete contents
				newdir = concat_path_file(directory, newdir);
				delete_contents(newdir, rootdev);
				free(newdir);
			}
			closedir(dir);

			// Directory should now be empty, zap it
			rmdir(directory);
		}
	} else {
		// It wasn't a directory, zap it
		unlink(directory);
	}
}

//---------------------------------------------------------
//	main
//---------------------------------------------------------

int switch_root_main(int argc, /*const */char **argv)
{
	char *newroot, *console = NULL;
	struct stat st;
	dev_t rootdev;

	if (argc != 3) {
		fprintf(stderr, "usage: %s new_root new_init\n", argv[0]);
		return 1;
	}
	argv++;
	newroot = *argv++;

	// Change to new root directory
	chdir(newroot);
	stat("/", &st);
	rootdev = st.st_dev;
	stat(".", &st);
	if (st.st_dev == rootdev) {
		fprintf(stderr, "New root, %s, must be a mountpoint!\n", newroot);
//		return 1;
	}
	if (getpid() != 1) {
		fprintf(stderr, "We must be PID 1!\n");
		return 1;
	}
	// Zap everything out of rootdev
	delete_contents("/", rootdev);

	// Overmount / with newdir and chroot into it
	if (mount(".", "/", NULL, MS_MOVE, NULL)) {
		// For example, fails when newroot is not a mountpoint
		fprintf(stderr, "Error moving root!\n");
		return 1;
	}
	chroot(".");
	// The chdir is needed to recalculate "." and ".." links
	chdir("/");

	// If a new console specified, redirect stdin/stdout/stderr to it
	if (console) {
		close(0);
		xopen(console, O_RDWR);
		xdup2(0, 1);
		xdup2(0, 2);
	}

	// Exec real init
	execv(argv[0], argv);

	fprintf(stderr, "Can't execute '%s'", argv[0]);
	return 1;
}
