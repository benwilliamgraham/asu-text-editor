#include "fstr.h"

// create a new fstr
fstr *fstr_open(char *filename) {
    // allocate file string
    fstr *f = malloc(sizeof(fstr));
    f->filename = filename;

    // attempt to open file
    FILE *file = fopen(filename, "r+");
    if (file == NULL)
        return NULL;

    // get size of file
    f->size = 0;
    while (fgetc(file) != EOF)
        f->size += 1;
    
    // reset file
    file = freopen(filename, "r+", file);
    
    // allocate and read file
    f->data = malloc(f->size);
    for (size_t index = 0; index < f->size; index++)
        f->data[index] = fgetc(file);
    
    fclose(file);

    return f;
}

// write an fstr back to file
void fstr_write(fstr *f) {
    FILE *file = fopen(f->filename, "w+");
    for (size_t index = 0; index < f->size; index++)
        fputc(f->data[index], file);
    fclose(file);
}

// close fstr
void fstr_close(fstr *f) {
    free(f->data);
    free(f);
}

// append char to fstr
void fstr_append(fstr *f, char c) {
    f->data = realloc(f->data, f->size + 1);
    f->data[f->size] = c;
    f->size += 1;
}

// insert char at index into fstr
void fstr_insert(fstr *f, char c, size_t index) {
    f->data = realloc(f->data, f->size + 1);
    memcpy(f->data + index + 1, f->data + index, f->size - index);
    f->data[index] = c;
    f->size += 1;
}

// remove char at index from fstr
void fstr_remove(fstr *f, size_t index) {
    char *new = malloc(f->size - 1);
    memcpy(new, f->data, index);
    memcpy(new + index, f->data + index + 1, f->size - index - 1);
    free(f->data);
    f->data = new;
    f->size -= 1;
}

// get char at index from fstr
char fstr_get(fstr *f, size_t index) {
    return f->data[index];
}

// get size of fstr
size_t fstr_size(fstr *f) {
    return f->size;
}