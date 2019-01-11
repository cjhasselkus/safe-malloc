# Safe Malloc

A set of library functions that check for common heap allocation and usage mistakes when using malloc, realloc, and free, or before any attempted memory access. These include mallocSafe, freeSafe, reallocSafe, and additional function, memcheckSafe, all of which are detailed below. This is done using a self balancing range tree data structure that stores currently allocated and freed block locations. If a faulty call is made, these routines output a descriptive error message and terminate the process.


## mallocSafe(size_t size)
After calling the *malloc* routine, mallocSafe enters the allocated virtual memory block into the range tree as a (start address, block size) pair. It also checks to make sure that the range tree doesn't already contain this address in one of the already allocated datablocks. Before insertion into the range tree, this function also deletes any freed blocks that would overlap with this new block from the range tree.


## freeSafe(void *ptr)
freeSafe first checks if this function is being called on a NULL pointer and outputs the corresponding error message. If it is called on a memory block that isn't already in the range tree, a separate error is printed. Finally, if the memory block is in the tree (and hasn't already been freed), *free* is called on the pointer. If a double free is detected, a corresponding error is output.


## reallocSafe(void *ptr, size_t size)
Similar to mallocSafe and freeSafe, this function checks that the pointer is actually in the tree, detects and outputs faulty access error messages, and calls *realloc* if the call to reallocSafe is valid. 


## memcheckSafe(void *ptr, size_t size)
This function's intended use is to be called before any heap memory is accessed. Before accessing heap memory allocated using mallocSafe, calling memcheckSafe would detect common memory access errors. This routine detects if the pointer being checked wasn't allocated with mallocSafe, if this pointer was allocated with mallocSafe but is already freeed, or if this pointer was allocated with mallocSafe, but it was allocated with a size smaller than the size being requested. 


## Authors

* **Carolyn Hasselkus** - Work done for CS537 : Introduction to Operation Systems at University of Wisconsin-Madison, taught by Professor Barton Miller.
