#ifndef MALLOC_H_
#define MALLOC_H_

/* mallocSafe   : Allocates the requested block of memory and records tuple for that memory block. */
void *mallocSafe(size_t size);

/* freeSafe     : Frees the requested block of memory if reasonable and if so, frees tuple for that memory block. */
void freeSafe(void *ptr);

/* reallocSafe  : If ptr is NULL, do a malloc, if size is 0, do a free, and otherwise, change memory allocation with
                 realloc and replaces tuple if necessary. */
void *reallocSafe(void *ptr, size_t size);

/* memcheckSafe : Checks if the address rage specified by the address of ptr + size are within a valid tuple range in the range tree already. */
void memcheckSafe(void *ptr, size_t size);

#endif // MALLOC_H_