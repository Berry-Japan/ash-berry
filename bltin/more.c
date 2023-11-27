//---------------------------------------------------------
//	more
//
//		(C)2010 Yuichiro Nakada
//---------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#define CURRENT_TTY "/dev/tty"
#define OFF_FMT "l"

/* It is perfectly ok to pass in a NULL for either width or for
 * height, in which case that value will not be set.  */
#include <termios.h>
#include <sys/ioctl.h>
int get_terminal_width_height(const int fd, int *width, int *height)
{
	struct winsize win = { 0, 0, 0, 0 };
	int ret = ioctl(fd, TIOCGWINSZ, &win);

	if (height) {
		if (!win.ws_row) {
			char *s = getenv("LINES");
			if (s) win.ws_row = atoi(s);
		}
		if (win.ws_row <= 1 || win.ws_row >= 30000)
			win.ws_row = 24;
		*height = (int) win.ws_row;
	}

	if (width) {
		if (!win.ws_col) {
			char *s = getenv("COLUMNS");
			if (s) win.ws_col = atoi(s);
		}
		if (win.ws_col <= 1 || win.ws_col >= 30000)
			win.ws_col = 80;
		*width = (int) win.ws_col;
	}

	return ret;
}

#if ENABLE_FEATURE_USE_TERMIOS
static int cin_fileno;
#include <termios.h>
#define setTermSettings(fd, argp) tcsetattr(fd, TCSANOW, argp)
#define getTermSettings(fd, argp) tcgetattr(fd, argp);

static struct termios initial_settings, new_settings;

static void set_tty_to_initial_mode(void)
{
	setTermSettings(cin_fileno, &initial_settings);
}

static void gotsig(int sig)
{
	putchar('\n');
	exit(EXIT_FAILURE);
}
#endif /* FEATURE_USE_TERMIOS */


int more_main(int argc, char **argv)
{
	int c, lines, input = 0;
	int please_display_more_prompt = 0;
	struct stat st;
	FILE *file;
	FILE *cin;
	int len, page_height;
	int terminal_width;
	int terminal_height;

	argv++;
	/* Another popular pager, most, detects when stdout
	 * is not a tty and turns into cat. This makes sense. */
	//if (!isatty(STDOUT_FILENO)) return catcmd(argc, argv);
	cin = fopen(CURRENT_TTY, "r");
	if (!cin) {
		cin = fopen("/dev/tty0", "r");
		if (!cin) return catcmd(argc, argv);
	}

#if ENABLE_FEATURE_USE_TERMIOS
	cin_fileno = fileno(cin);
	getTermSettings(cin_fileno, &initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;
	new_settings.c_lflag &= ~ECHO;
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	setTermSettings(cin_fileno, &new_settings);
	atexit(set_tty_to_initial_mode);
	signal(SIGINT, gotsig);
	signal(SIGQUIT, gotsig);
	signal(SIGTERM, gotsig);
#endif
	please_display_more_prompt = 2;

	do {
		file = stdin;
		if (*argv) {
			//file = fopen_or_warn(*argv, "r");
			file = fopen(*argv, "r");
			if (!file) continue;
		}
		st.st_size = 0;
		fstat(fileno(file), &st);

		please_display_more_prompt &= ~1;
		/* never returns w, h <= 1 */
		get_terminal_width_height(fileno(cin), &terminal_width, &terminal_height);
		terminal_width -= 1;
		terminal_height -= 1;

		len = 0;
		lines = 0;
		page_height = terminal_height;
		while ((c = getc(file)) != EOF) {

			if ((please_display_more_prompt & 3) == 3) {
				len = printf("--More-- ");
				if (/*file != stdin &&*/ st.st_size > 0) {
					len += printf("(%d%% of %"OFF_FMT"d bytes)",
						(int) (ftello(file)*100 / st.st_size),
						st.st_size);
				}
				fflush(stdout);

				/*
				 * We've just displayed the "--More--" prompt, so now we need
				 * to get input from the user.
				 */
				input = getc(cin);
#if !ENABLE_FEATURE_USE_TERMIOS
				printf("\033[A"); /* up cursor */
#endif
				/* Erase the "More" message */
				printf("\r%*s\r", len, "");
				len = 0;
				lines = 0;
				/* Bottom line on page will become top line
				 * after one page forward. Thus -1: */
				page_height = terminal_height - 1;
				please_display_more_prompt &= ~1;

				if (input == 'q') goto end;
			}

			/*
			 * There are two input streams to worry about here:
			 *
			 * c     : the character we are reading from the file being "mored"
			 * input : a character received from the keyboard
			 *
			 * If we hit a newline in the _file_ stream, we want to test and
			 * see if any characters have been hit in the _input_ stream. This
			 * allows the user to quit while in the middle of a file.
			 */
			if (c == '\n') {
				/* increment by just one line if we are at
				 * the end of this line */
				if (input == '\n')
					please_display_more_prompt |= 1;
				/* Adjust the terminal height for any overlap, so that
				 * no lines get lost off the top. */
				if (len >= terminal_width) {
					int quot, rem;
					quot = len / terminal_width;
					rem  = len - (quot * terminal_width);
					page_height -= (quot - 1);
					if (rem) page_height--;
				}
				if (++lines >= page_height) {
					please_display_more_prompt |= 1;
				}
				len = 0;
			}
			/*
			 * If we just read a newline from the file being 'mored' and any
			 * key other than a return is hit, scroll by one page
			 */
			putc(c, stdout);
			/* My small mind cannot fathom tabs, backspaces,
			 * and UTF-8 */
			len++;
		}
		fclose(file);
		fflush(stdout);
	} while (*argv && *++argv);
 end:
	//printf("\n%dx%d\n", terminal_width, terminal_height);
	return 0;
}
