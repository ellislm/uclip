# μclip: The Tiny CLI Clipboard

μclip is a simple, cli-based clipboard that operates on `stdin` and `stdout`. Its primary purpose is to provide a simple clipboard that can interface between `tmux`, `vim`, and system clipboards.

## Install

Compile the uclip source file into a binary using your chosen C compiler. A few examples:

```sh
$ gcc -Wall -Wextra uclip.c -o uclip && \
  ln -s uclip /path/to/local/bin

# or:
$ clang -Wall -Wextra uclip.c -o uclip && \
  ln -s uclip /path/to/local/bin
```

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

For vim, you can yank and paste to uclip using ex commands:

```
# Copy entire register
:%w !uclip -i

# Or visual selection:
:'<,'>w !uclip -i

# To paste contents at line N:
:Nr !uclip -o

# To overwrite selection with clipboard contents:
:'<,'>!uclip -o
```

(Note: for nvim only) Alternatively, you  can replace the system registers by adding the following to your `vimrc`:

```
let g:clipboard = {
    'name': 'uclip',
    'copy': {
        '+': ['uclip', '-i'],
        '*': ['uclip', '-i'],
    },
    'paste': {
        '+': ['uclip', '-o'],
        '*': ['uclip', '-o'],
    },
    'cache_enabled': 1,
}
```

### Tmux

#### Yank

For `vi` keybinds, add the following to your `tmux.conf`:

```sh
bind -Tcopy-mode-vi y               send -X copy-pipe-and-cancel 'uclip -i'
bind -Tcopy-mode-vi C-j               send -X copy-pipe-and-cancel 'uclip -i'
bind -Tcopy-mode-vi Enter             send -X copy-pipe-and-cancel 'uclip -i'
bind -Tcopy-mode-vi MouseDragEnd1Pane send -X copy-pipe-and-cancel 'uclip -i'
set-option -s set-clipboard off
```

or for `emacs` keybinds, add the following:

```sh
bind -Tcopy-mode C-w               send -X copy-pipe-and-cancel 'uclip -i'
bind -Tcopy-mode M-w               send -X copy-pipe-and-cancel 'uclip -i'
bind -Tcopy-mode MouseDragEnd1Pane send -X copy-pipe-and-cancel 'uclip -i'
```

Note: this only works for `tmux 2.4+`. For earlier versions, see the [official documentation](https://github.com/tmux/tmux/wiki/Clipboard#how-to-configure---tmux-23-and-earlier)

#### Paste

Coming soon

### OS

Linux, macOS, and \*BSD coming soon!
