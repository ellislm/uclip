# μclip: The Tiny CLI Clipboard

μclip is a simple, cli-based clipboard that operates on `stdin` and `stdout`. Its primary purpose is to provide a simple clipboard that can interface between `tmux`, `vim`, and system clipboards.

## Usage

```sh
# Write content to clipboard:
$ echo "Hello World" | uclip -i

# Print contents to stdout:
$ uclip -o

# Read file contents, but do not print any errors if they occur:
$ uclip -q -i < README.md
```

## Integrations

### Vim

Coming soon!

### Tmux

Coming soon!

### OS

Linux, macOS, and *BSD coming soon!
