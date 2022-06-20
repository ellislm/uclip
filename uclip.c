#include <sys/errno.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_FNAME_SIZE 512
#define CLIPPERMS (S_IWUSR|S_IRUSR)

bool stdin_empty(void);
void check_permissions(char *path);
void create_clipboard(char *path);
void get_clipboard_path(char *);
FILE *open_clipboard(char *, char *);
int read_clipboard(FILE *);
int write_clipboard(FILE *);
void parse_args(char *, int, char **);
void usage(void);

/*
 * Returns a bool indicating whether or not `stdin` is empty. This can be used
 * to prevent blocking calls to `*read` functions when no input is provided.
 */
bool
stdin_empty(void)
{
	bool is_empty;

	is_empty = true;

	if (fseek(stdin, 0, SEEK_END) < 0) {
		err(EX_IOERR, "Unable to read stdin");
	}
	if (ftell(stdin) > 0) {
		is_empty = false;
		rewind(stdin);
	}
	return is_empty;
}

/*
 * Ensure that clipboard mode is user read/write only. Will also attempt to 
 * create if it does not exist.
 */
void
check_permissions(char *path)
{
	struct stat file_stats;
	mode_t file_permissions;

	if (stat(path, &file_stats) < 0) {
		if (errno == ENOENT) {
			/* File does not exist, so let's create it */
			create_clipboard(path);
		} else {
			err(EX_UNAVAILABLE, "Unable to verify clipboard permissions");
		}
	} else {
    /* Mask out all except permissions */
    file_permissions = file_stats.st_mode & ALLPERMS;

    if (file_permissions != CLIPPERMS) {
      warnx("Incorrect clipboard permissions. Setting to user rw only");
      chmod(path, CLIPPERMS);
    }
	}
}

/*
 * Create the clipboard with the appropriate permissions.
 */
void
create_clipboard(char *path)
{
	int fd;

	if ((fd = open(path, O_RDWR | O_CREAT)) < 0) {
		err(EX_UNAVAILABLE, "Unable to create clipboard at %s", path);
	}

	if (fchmod(fd, CLIPPERMS) < 0) {
		err(EX_UNAVAILABLE, "Unable to set clipboard permissions");
	}

  if(close(fd)){
    err(EX_UNAVAILABLE, "Error closing clipboard");
  }

}
/*
 * Validates the clipboard path and writes it to `clipboard_dir`. Should a
 * failure occur, the function will terminate the process without returning.
 */
void
get_clipboard_path(char *clipboard_dir)
{
	int bytes_written;

	bytes_written = snprintf(clipboard_dir, MAX_FNAME_SIZE - 1,
	    "/tmp/uclip-%i", getuid());

	if (bytes_written >= MAX_FNAME_SIZE) {
		errx(EX_USAGE,
		    "Failed to write clipboard name. "
		    "Resultant string exceeds %i",
		    MAX_FNAME_SIZE);
	} else if (bytes_written < 0) {
		err(EX_UNAVAILABLE, "Error writing clipboard name");
	}
}

/*
 *  Prints command line instructions and terminates the process;
 */
void
usage(void)
{
	fprintf(stderr, "usage: %s -i/-o\n", getprogname());
	exit(EX_USAGE);
}

/*
 * Returns the filestream for the clipboard specified at `clipboard_path`. If a
 * failure occurs, the function terminates the process without returning.
 */
FILE *
open_clipboard(char *clipboard_path, char *mode)
{
	FILE *clipboard;
	clipboard = fopen(clipboard_path, mode);

	if (clipboard == NULL) {
		err(EX_UNAVAILABLE, "Error opening clipboard at %s", clipboard_path);
	}

	return clipboard;
}

/*
 * Sets the appropriate clipboard `fopen` `mode` to either `(r)ead` or `(w)rite)
 * based on the arguments passed to the binary.
 */
void
parse_args(char *mode, int argc, char *argv[])
{
	int ch;
	bool mode_set;

	mode_set = false;
	while ((ch = getopt(argc, argv, "ioq")) != -1) {
		switch (ch) {
		case 'i':
			if (mode_set) {
				usage();
			}
			strcpy(mode, "w");
			mode_set = true;

			break;
		case 'o':
			if (mode_set) {
				usage();
			}
			strcpy(mode, "r");
			mode_set = true;
			break;
		case '?':
			/* FALLTHROUGH */
		default:
			usage();
		}
	}

	if (!mode_set) {
		usage();
	}
}

/*
 * Print the contents of `clipboard` to `stdout` and return the number of bytes
 * printed. Function will terminate the process without returning if errors are
 * encountered.
 */
int
read_clipboard(FILE * clipboard)
{
	char buffer[BUFFER_SIZE];
	size_t bytes_read, total_bytes;

	bytes_read = BUFFER_SIZE + 1;
	total_bytes = 0;

	while (bytes_read >= BUFFER_SIZE) {
		bytes_read = fread(buffer, 1, BUFFER_SIZE, clipboard);

		if (fwrite(buffer, 1, bytes_read, stdout) < bytes_read) {
			err(EX_UNAVAILABLE, "Error writing to stdout");
		}
		total_bytes += bytes_read;
	}

	if (ferror(stdin)) {
		err(EX_UNAVAILABLE, "Error reading clipboard");
	}
	return total_bytes;
}

/*
 * Read the contents of `stdin`, write to `clipboard` and return the number of
 * bytes written. Function will terminate the process without returning if
 * errors are encountered.
 */
int
write_clipboard(FILE * clipboard)
{
	char buffer[BUFFER_SIZE];
	size_t bytes_read, total_bytes;

	bytes_read = BUFFER_SIZE + 1;
	total_bytes = 0;

	if (stdin_empty()) {
		return 0;
	}
	while (bytes_read >= BUFFER_SIZE) {
		bytes_read = fread(buffer, 1, BUFFER_SIZE, stdin);
		fwrite(buffer, 1, bytes_read, clipboard);
		total_bytes += bytes_read;
	}

	if (ferror(stdin)) {
		err(EX_UNAVAILABLE, "Error writing clipboard");
	}
	return total_bytes;
}

/*
 * uclip is a tiny cli-based clipboard that reads content from stdin and writes
 * clipboard contents stdout.
 */
int
main(int argc, char *argv[])
{
	FILE *clipboard;
	char mode[3];
	char clipboard_path[MAX_FNAME_SIZE];

	parse_args(mode, argc, argv);
	get_clipboard_path(clipboard_path);
	check_permissions(clipboard_path);
	clipboard = open_clipboard(clipboard_path, mode);

	if (*mode == 'r') {
		read_clipboard(clipboard);
	} else {
		write_clipboard(clipboard);
	}

	return 0;
}
