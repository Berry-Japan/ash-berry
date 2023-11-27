//---------------------------------------------------------
//	dmesg
//
//		(C)2010 Yuichiro Nakada
//---------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <sys/klog.h>

int dmesg_main(int argc, char *argv[])
{
	int len;
	char *buf;

	len = /*(flags & 2) ? xatoul_range(size, 2, INT_MAX) : */16384;
	buf = /*x*/malloc(len);
	if (0 > (len = klogctl(3 /*+ (flags & 1)*/, buf, len))) {
		fprintf(stderr, "Error at klogctl\n", argv[0]);
		return 1;
	}

	// Skip <#> at the start of lines, and make sure we end with a newline.
	/*if (ENABLE_FEATURE_DMESG_PRETTY) */{
		int last = '\n';
		int in;

		for (in=0; in<len;) {
			if (last == '\n' && buf[in] == '<') in += 3;
			else putchar(last = buf[in++]);
		}
		if (last != '\n') putchar('\n');
	}/* else {
		write(1, buf, len);
		if (len && buf[len-1]!='\n') putchar('\n');
	}

	if (ENABLE_FEATURE_CLEAN_UP) */free(buf);

	return 0;
}
