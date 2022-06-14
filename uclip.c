#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>

#define MAX_NUM_BYTES 1024
#define BUFFER_SIZE 1024
#define MAX_FNAME_SIZE 256

/* TODO:
 * - default path for clipboard
 * - file locking
 */

int get_clipboard_path(char *clipboard);
FILE *open_clipboard(char *clipboard_path, char *mode);
int write_clipboard(FILE *clipboard);

int get_clipboard_path(char *clipboard_dir) {
  char *home_dir;
  int bytes_written;
  uid_t uid;

  uid = getuid();
  home_dir = getpwuid(uid)->pw_dir;
  bytes_written = snprintf(clipboard_dir, MAX_FNAME_SIZE - 1,
                               "%s/tmp/uclipb%i", home_dir, uid);

  if (bytes_written >= MAX_FNAME_SIZE) {
    fprintf(stderr,
            "Failed to write clipboard name. "
            "Resultant string exceeds %i",
            MAX_FNAME_SIZE);
  } else if (bytes_written < 0) {
    perror("Error writing clipboard name");
  }

  return bytes_written;
}

/*
 * Returns file descriptor to clipboard file.
 * @param path a string path to clipboard file
 */
FILE *
open_clipboard(char *clipboard_path, char *mode) {
  FILE *clipboard;

  clipboard = fopen(clipboard_path, mode);

  if (clipboard == NULL) {
    puts(clipboard_path);
    perror("Error opening clipboard");
  }
  /*
   * If path does not exist, create with user rw-only perms
   * flock open the uclipUID file
   * return the fd
   */
  return clipboard;
}

int
write_clipboard(FILE *clipboard) {
  char buffer[BUFFER_SIZE];
  size_t bytes_read, total_bytes;

  bytes_read = BUFFER_SIZE + 1;
  total_bytes= 0;

  while (bytes_read >= BUFFER_SIZE) {
    bytes_read = fread(buffer, 1, BUFFER_SIZE, stdin);
    fwrite(buffer, 1, bytes_read, clipboard);
    total_bytes += bytes_read;
  }

  if (ferror(stdin)) {
    perror("Error writing clipboard");
    return EXIT_FAILURE;
  }

  return 0;
}

int
main(int argc, char *argv[]) {
  /* If path is not given, set default path to /tmp/uclipUID */
  FILE *clipboard;
  char clipboard_path[MAX_FNAME_SIZE];

  if (((get_clipboard_path(clipboard_path)) < 0) ||
      ((clipboard = open_clipboard(clipboard_path, "w+")) == NULL)) {
    return EXIT_FAILURE;
  }

  return write_clipboard(clipboard);
}
