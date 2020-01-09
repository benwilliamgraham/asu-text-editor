/*
 * Asu
 * Asymilated Scrolling with Unicode support
 *
 * Ben Graham
 * benwilliamgraham@gmail.com
 */
#include "fstr.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// cursor
struct cursor {
  // data pointer
  size_t ptr;

  // x, y position
  long x, y;
} cur;

// frame
struct frame {
  // x, y position
  long x, y;
} frm;

// terminal dimensions
struct winsize dimensions;

// file string
fstr *data;

// state
struct termios previous;
struct termios editing;

// highlighting
enum Highlight { NUMBER, NAME, SYMBOL, OTHER } highlight = OTHER;

// draw character
void draw(char c) {
  // currently number
  if (highlight == NUMBER &&
      ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
       (c >= 'A' && c <= 'Z') || c == '.'))
    printf("%c", c);

  // currently name
  else if (highlight == NAME &&
           ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') || c == '_'))
    printf("%c", c);

  else if (highlight == SYMBOL &&
           ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') ||
            (c >= '[' && c <= '`') || c == '~'))
    printf("%c", c);

  // currently other
  else {
    highlight = OTHER;

    // if is number
    if (c >= '0' && c <= '9') {
      highlight = NUMBER;
      printf("\033[93m%c", c);
    }

    // is name
    else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
      highlight = NAME;
      printf("\033[0m%c", c);
    }

    // is whitespace
    else if (c == ' ')
      printf(" ");

    // is symbol
    else {
      highlight = SYMBOL;
      printf("\033[94m%c", c);
    }
  }
}

// throw error
void error(char *message, char *source) {
  printf("\033[91;1merror:\033[0m %s: \033[90m%s\033[0m\n", message, source);
  exit(EXIT_FAILURE);
}

// move cursor up
void up() {
  if (cur.y > 0) {
    cur.ptr -= 1;
    while (fstr_get(data, cur.ptr) != '\n')
      cur.ptr -= 1;
    cur.ptr -= 1;
    long size = 1;
    while (cur.ptr > 0 && fstr_get(data, cur.ptr) != '\n') {
      cur.ptr -= 1;
      size += 1;
    }
    cur.ptr += (cur.x > size) ? size : cur.x + 1;
  } else
    cur.ptr = 0;
}

// move cursor down
void down() {
  while (cur.ptr < fstr_size(data) && fstr_get(data, cur.ptr) != '\n')
    cur.ptr += 1;
  if (cur.ptr < fstr_size(data))
    cur.ptr += 1;
  long x = 0;
  while (cur.ptr < fstr_size(data) && fstr_get(data, cur.ptr) != '\n' &&
         x < cur.x) {
    cur.ptr += 1;
    x += 1;
  }
}

// move cursor left
void left() {
  if (cur.ptr > 0)
    cur.ptr -= 1;
}

// move cursor right
void right() {
  if (cur.ptr < fstr_size(data))
    cur.ptr += 1;
}

// insert character at cursor
void insert(char c) {
  if (cur.ptr == fstr_size(data))
    fstr_append(data, c);
  else
    fstr_insert(data, c, cur.ptr);
  cur.ptr += 1;
}

// backspace
void backspace() {
  if (cur.ptr > 0) {
    fstr_remove(data, cur.ptr - 1);
    cur.ptr -= 1;
  }
}

// render current
void render() {
  // read terminal dimensions
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &dimensions);

  // reset cursor position
  if (cur.x > 0)
    printf("\033[%ldD", cur.x);
  if (cur.y - frm.y > 0)
    printf("\033[%ldA", cur.y - frm.y);

  size_t p = 0;
  long x = 0, y = 0, margin = 2;

  // calculate cx and cy
  while (p < cur.ptr) {
    if (fstr_get(data, p) == '\n') {
      x = 0;
      y += 1;
    } else if (fstr_get(data, p) == '\t') {
      x += 4;
    } else
      x += 1;
    p += 1;
  }
  cur.x = x;
  cur.y = y;

  // calculate frame shift
  frm.x = (cur.x - dimensions.ws_col + margin > 0)
              ? cur.x - dimensions.ws_col + margin
              : 0;
  frm.y = (cur.y - dimensions.ws_row + margin > 0)
              ? cur.y - dimensions.ws_row + margin
              : 0;

  // display results
  x = 0, y = 0, p = 0;

  // pass non-visible
  while (y < frm.y) {
    if (fstr_get(data, p) == '\n')
      y += 1;
    p += 1;
  }

  // draw
  while (p < fstr_size(data) && y <= frm.y + dimensions.ws_row - margin) {
    if (fstr_get(data, p) == '\n') {
      highlight = OTHER;
      printf("\033[K\n");
      y += 1;
      x = 0;
    } else if (fstr_get(data, p) == '\t') {
      printf("    ");
      x += 4;
    } else {
      if (x >= frm.x && x < frm.x + dimensions.ws_col) {
        draw(fstr_get(data, p));
      }
      x += 1;
    }
    p += 1;
  }

  // clear rest of screen
  printf("\033[0m\033[K");
  while (y <= frm.y + dimensions.ws_row - margin) {
    printf("\033[K\n");
    y += 1;
  }

  // reset cusor position
  if (x > 0)
    printf("\033[%ldD", x);
  if (y > 0)
    printf("\033[%ldA", y);

  // set to current character
  if (cur.x - frm.x > 0)
    printf("\033[%ldC", cur.x - frm.x);
  if (cur.y - frm.y > 0)
    printf("\033[%ldB", cur.y - frm.y);

  // display
  fflush(stdout);
}

// main
int main(int argc, char **argv) {
  // create editor
  cur.x = 0, cur.y = 0, cur.ptr = 0, frm.x = 0, frm.y = 0;

  // load file
  if (argc != 2)
    error("expected format", "asu [filename]");
  char *filename = argv[1];
  if ((data = fstr_open(filename)) == NULL)
    error("unnable to open file", filename);

  // make terminal editable
  tcgetattr(STDIN_FILENO, &previous);
  editing = previous;
  editing.c_iflag &= ~IXON;
  editing.c_lflag &= ~(ECHO | ICANON);
  editing.c_cc[VMIN] = 0;
  editing.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &editing);
  render();

  // edit
  char c;
  while (true) {
    // no update
    if (!read(STDIN_FILENO, &c, 1))
      continue;

    // save and exit (ctrl-S)
    else if (c == 19) {
      fstr_write(data);
      break;
    }

    // escape
    else if (c == 27) {

      // escape character
      if (read(STDIN_FILENO, &c, 1)) {
        // control character
        if (c == '[') {
          read(STDIN_FILENO, &c, 1);

          // arrow keys
          if (c == 'A')
            up();
          else if (c == 'B')
            down();
          else if (c == 'C')
            right();
          else if (c == 'D')
            left();
        }
      }

      // escape key
      else
        break;
    }

    // backspace
    else if (c == 127) {
      if (cur.ptr > 0)
        backspace();
    }

    // character
    else {
      if (c == 13)
        c = '\n';
      insert(c);
    }
    render();
  }

  // set position to end
  cur.ptr = fstr_size(data);
  render();
  printf("\n");
  fstr_close(data);

  // reset terminal
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &previous);

  return 0;
}