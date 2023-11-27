//---------------------------------------------------------
//	losetup - setup and control loop devices
//
//		(C)2003 NAKADA
//---------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "loop.h"


//---------------------------------------------------------
//	crypt table
//---------------------------------------------------------

struct crypt_type_struct {
	int id;
	char *name;
} crypt_type_tbl[] = {
	{ LO_CRYPT_NONE,"no" },
	{ LO_CRYPT_NONE,"none" },
	{ LO_CRYPT_XOR,	"xor" },
	{ LO_CRYPT_DES,	"DES" },
	{ -1,		NULL }
};


//---------------------------------------------------------
//	get crypt name
//---------------------------------------------------------

static char *crypt_name(int id)
{
	int i;

	for (i = 0; crypt_type_tbl[i].id != -1; i++) {
		if (id == crypt_type_tbl[i].id) return crypt_type_tbl[i].name;
	}

	return "undefined";
}


//---------------------------------------------------------
//	get crypt id
//---------------------------------------------------------

static int crypt_type(const char *name)
{
	int i;

	for (i = 0; crypt_type_tbl[i].id != -1; i++)
		if (!strcasecmp(name, crypt_type_tbl[i].name))
			return crypt_type_tbl[i].id;
	return -1;
}


//---------------------------------------------------------
//	show loop device
//---------------------------------------------------------

static int show_loop (char *device) {
	struct loop_info loopinfo;
	int fd;

	if ((fd = open(device, O_RDONLY)) < 0) {
		int errsv = errno;
		fprintf(stderr, "loop: can't open device %s: %s\n",
			device, strerror(errsv));
		return 2;
	}
	if (ioctl(fd, LOOP_GET_STATUS, &loopinfo) < 0) {
		int errsv = errno;
		fprintf(stderr, "loop: can't get info on device %s: %s\n",
			device, strerror(errsv));
		close(fd);
		return 1;
	}
	printf("%s: [%04x]:%ld (%s) offset %d, %s encryption\n",
		device, (int)loopinfo.lo_device, loopinfo.lo_inode,
		loopinfo.lo_name, loopinfo.lo_offset,
		crypt_name(loopinfo.lo_encrypt_type));
	close(fd);

	return 0;
}


//---------------------------------------------------------
//	set loop
//---------------------------------------------------------

int set_loop(const char *device, const char *file, int offset,
	      const char *encryption, int *loopro)
{
	struct loop_info loopinfo;
	int	fd, ffd, mode, i;
	char	*pass;

	mode = *loopro ? O_RDONLY : O_RDWR;
	if ((ffd = open(file, mode)) < 0 && !*loopro
	    && (errno != EROFS || (ffd = open(file, mode = O_RDONLY)) < 0)) {
		perror(file);
		return 1;
	}
	if ((fd = open(device, mode)) < 0) {
		perror(device);
		return 1;
	}
	*loopro = (mode == O_RDONLY);

	// Warning !!
	if (*loopro) printf("losetup: Warning: Open %s as Using Read Only Mode.\n", device);

	// init loopinfo
	memset(&loopinfo, 0, sizeof(loopinfo));
	strncpy(loopinfo.lo_name, file, LO_NAME_SIZE);
	loopinfo.lo_name[LO_NAME_SIZE-1] = 0;

	if (encryption && (loopinfo.lo_encrypt_type = crypt_type(encryption)) < 0) {
		fprintf(stderr, "Unsupported encryption type %s\n", encryption);
		return 1;
//	} else {
//		loopinfo.lo_encrypt_type=LO_CRYPT_NONE;
		// because init.
	}

	loopinfo.lo_offset = offset;
	switch (loopinfo.lo_encrypt_type) {
	case LO_CRYPT_NONE:
		loopinfo.lo_encrypt_key_size = 0;
		break;
	case LO_CRYPT_XOR:
		pass = getpass("Password: ");
		strncpy(loopinfo.lo_encrypt_key, pass, LO_KEY_SIZE);
		loopinfo.lo_encrypt_key[LO_KEY_SIZE-1] = 0;
		loopinfo.lo_encrypt_key_size = strlen(loopinfo.lo_encrypt_key);
		break;
	case LO_CRYPT_DES:
		pass = getpass("Password: ");
		strncpy(loopinfo.lo_encrypt_key, pass, 8);
		loopinfo.lo_encrypt_key[8] = 0;
		loopinfo.lo_encrypt_key_size = 8;
		pass = getpass("Init (up to 16 hex digits): ");
		for (i = 0; i < 16 && pass[i]; i++)
			if (isxdigit(pass[i]))
				loopinfo.lo_init[i >> 3] |= (pass[i] > '9' ?
				    (islower(pass[i]) ? toupper(pass[i]) :
				    pass[i])-'A'+10 : pass[i]-'0') << (i & 7)*4;
			else {
				fprintf(stderr, "Non-hex digit '%c'.\n", pass[i]);
				return 1;
			}
		break;
	default:
		fprintf(stderr,
			"Don't know how to get key for encryption system %d\n",
			loopinfo.lo_encrypt_type);
		return 1;
	}
	if (ioctl(fd, LOOP_SET_FD, ffd) < 0) {
		perror("ioctl: LOOP_SET_FD");
		return 1;
	}
	if (ioctl(fd, LOOP_SET_STATUS, &loopinfo) < 0) {
		(void) ioctl(fd, LOOP_CLR_FD, 0);
		perror("ioctl: LOOP_SET_STATUS");
		return 1;
	}
	close(fd);
	close(ffd);

	return 0;
}


//---------------------------------------------------------
//	del loop
//---------------------------------------------------------

int del_loop(const char *device)
{
	int fd;

	if ((fd = open(device, O_RDONLY)) < 0) {
		perror(device);
		return 1;
	}
	if (ioctl(fd, LOOP_CLR_FD, 0) < 0) {
		perror("ioctl: LOOP_CLR_FD");
		return 1;
	}

	return 0;
}


//---------------------------------------------------------
//	show usage
//---------------------------------------------------------

static int usage(char *prog)
{
	fprintf(stderr, "Usage:\n\
  %s loop_device                                      # give info\n\
  %s -d loop_device                                   # delete\n\
  %s [ -e encryption ] [ -o offset ] loop_device file # setup\n\
  This is a builtin command by NAKADA.\n",
		prog, prog, prog);

	return -1;
}


//---------------------------------------------------------
//	losetup
//---------------------------------------------------------

int losetup_main(int argc, char **argv)
{
	char *encryption;
//	int delete = 0;
	int offset = 0;
	int opt;

	int i;
	char *dv, *mp;

/*	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);*/

	encryption = NULL;

	dv=mp=NULL;
	opt=0;
	if (argc < 2) return usage(argv[0]);
	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i],"-d")) {
			// delete
//			delete = 1;
			i++;
			if (i==argc || encryption || offset) return usage(argv[0]);
			return del_loop(argv[i]);
		} else if (!strcmp(argv[i],"-e")) {
			// encryption
			i++;
			if (i==argc) return usage(argv[0]);
			encryption = argv[i];
		} else if (!strcmp(argv[i],"-o")) {
			// offset
			i++;
			if (i==argc) return usage(argv[0]);
			if (sscanf(argv[i], "%d", &offset) != 1) usage(argv[0]);
		} else if (!dv) {
			dv=argv[i];
		} else if (!mp) {
			mp=argv[i];
		} else {
			// Don't understand
			return usage(argv[0]);
		}
	}

	if (!mp) return show_loop(dv);

	return set_loop(dv, mp, offset, encryption, &opt);

/*	while ((opt = getopt(argc, argv, "de:o:")) != -1) {
		switch (opt) {
		case 'd':
			delete = 1;
			break;

		case 'e':
			encryption = optarg;
			break;

		case 'o':
			if (sscanf(optarg, "%d", &offset) != 1) usage(argv[0]);
			break;

		default:
			return usage(argv[0]);
		}
	}

	if ((delete && (argc != optind+1 || encryption || offset)) ||
		(!delete && (argc < optind+1 || argc > optind+2))) return usage(argv[0]);
	if (argc == optind+1) {
		if (delete) return del_loop(argv[optind]);
		else return show_loop(argv[optind]);
	}

	return set_loop(argv[optind], argv[optind+1], offset, encryption, &opt);*/
}
