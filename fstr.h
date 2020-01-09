/*
 * file string
 * load a file and edit it as a string
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

// file string structure
typedef struct {
    char *data;
    size_t size;
    char *filename;
} fstr;

// create a new fstr
fstr *fstr_open(char *filename);

// write an fstr back to file
void fstr_write(fstr *f);

// close fstr
void fstr_close(fstr *f);

// append char to fstr
void fstr_append(fstr *f, char c);

// insert char at index into fstr
void fstr_insert(fstr *f, char c, size_t index);

// remove char at index from fstr
void fstr_remove(fstr *f, size_t index);

// get char at index from fstr
char fstr_get(fstr *f, size_t index);

// get size of fstr
size_t fstr_size(fstr *f);