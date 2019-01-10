#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "Safemalloc.h"
#include "rangeTree.h"

//static tree root
static node * root = NULL;


/* mallocSafe */
void *mallocSafe(size_t size)
{
    if(size == 0)
    {
        fprintf(stderr, "Warning: Allocating memory of size 0.\n");
    }
    
    void* pointer = (void*)malloc(size); //pointer holds the address --> printf("address of this pointer is: %p\n", &pointer); //can compare simply with < and >
                                         //we already have the size 
                                         //both of these need to be added to a tuple to go into the range tree
    
    if(!pointer) //check that malloc was successful
    {
        fprintf(stderr, "Error: Memory allocaion failed. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        //make sure this node isn't already in the tree (it shouldn't be!)
        if(checkTreeContainsPtr(root, pointer, 0))
        {
            fprintf(stderr, "Error: mallocSafe attempted to allocate an already allocated memory block.\n");
            fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)pointer);
            fprintf(stderr, "       Exiting.\n");
            exit(-1);
        }
        
        //add this node to the range tree
        node * nodeToDelete = NULL;
        checkTreeBlockBounds(root, pointer, size, &nodeToDelete); //identify the nodes to delete and place them into the nodesToDelete list
        
        //iterate through nodesToDelete and delete... the... nodes....... to delete :-)
        while(nodeToDelete != NULL)
        {
            root = removeNode(root, nodeToDelete->addrRange->start);
            nodeToDelete = nodeToDelete->right;
        }
        
        //now that the tree is cleared of any old freed overlapping nodes, you can continue 
        root = insertNode(root, pointer, size);    
        return pointer; 
    }
}




/* freeSafe */
void freeSafe(void *ptr)
{  
    if(ptr == NULL)
    {
        fprintf(stderr, "Error: freeSafe is called on a null pointer.\n");
        fprintf(stderr, "       Exiting.\n");
        exit(-1);
    }
    
    node * matchingNode = checkTreeContainsPtr(root, ptr, 1); //will check errors
    if(matchingNode)
    {
        //you'll need mark the node as free
        matchingNode->freed = 1; 
        
        //call the real free
        free(ptr);
    }
    else //the node wasn't found
    {
        //if this wasn't the case, this address just isn't in the tree
        fprintf(stderr, "Error: freeSafe is called on a pointer that was not allocated with mallocSafe.\n");
        fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)ptr);
        fprintf(stderr, "       Exiting.\n");
        exit(-1);
    }
}



/* reallocSafe  - change size of this ptr in the range tree*/
void *reallocSafe(void *ptr, size_t size)
{
    if(!ptr) //if NULL
    {
        return mallocSafe(size);
    }
    else if(size == 0)
    {
        fprintf(stderr, "Warning: reallocSafe called with a size of 0.\n");
        //follow freeSafe's method
        
        
        node * matchingNode = checkTreeContainsPtr(root, ptr, 2);    
        if(matchingNode)
        {
            //you'll need mark the node as free
            matchingNode->freed = 1; 
            
            //call the real free
            return realloc(ptr,0);
        }
        else //the node wasn't found
        {        
            //if this wasn't the case, this address just isn't in the tree
            fprintf(stderr, "Error: reallocSafe is called on a pointer that was not allocated with mallocSafe.\n");
            fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)ptr);
            fprintf(stderr, "       Exiting.\n");
            exit(-1);
        }
    }
    else
    {
        //check tree to make sure it actually contains this pointer to reallocate to begin with 
        if(!checkTreeContainsPtr(root, ptr, 2))
        {
            fprintf(stderr, "Error: reallocSafe call made on a pointer that was not allocated by mallocSafe.\n");
            fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)ptr);
            fprintf(stderr, "       Exiting.\n");
            exit(-1);
        }
        
        //call realloc
        void * pointer = realloc(ptr, size);
        
        //need to do the same tree update we did for malloc here!
        if(!pointer) //check that realloc was successful
        {
            fprintf(stderr, "Error: Memory reallocaion failed. Exiting.\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            //add this node to the range tree          
            
            node * nodeToDelete = NULL;
            
            checkTreeBlockBounds(root, pointer, size, &nodeToDelete); //identify the nodes to delete and place them into the nodesToDelete list
                                                                     //most importantly, this will remove the node that we are reallocating!
            
            //iterate through nodesToDelete and delete... the... nodes....... to delete :-)
            while(nodeToDelete != NULL)
            {
                root = removeNode(root, nodeToDelete->addrRange->start);
                
                //free node that you just deleted
                node * tempNode = nodeToDelete->right;
                free(nodeToDelete->addrRange);
                free(nodeToDelete);
                nodeToDelete = NULL;
                
                nodeToDelete = tempNode;
            }
            
            
            //now that the tree is cleared of any old freed overlapping nodes, you can continue 
            root = insertNode(root, pointer, size);
            return pointer;
        }
    }

}

/* memcheckSafe - check that this memory range is contained within the tree*/
void memcheckSafe(void *ptr, size_t size)
{
    //check if the tree contains exactly 1 interval that starts with an address >= ptr and ends at an 
    //address <= size so this memory block is neatly fitting in exactly one memory block
    
    int errorNo = checkTreeContainsInterval(root, ptr, size);
    if(errorNo == -1) // -1 means this pointer isn't contained in the tree
    {
        fprintf(stderr, "Error: memcheckSafe identified a faulty memory access.\n");
        fprintf(stderr, "       Faulty call was on unallocated pointer with address %p\n", (void*)ptr);
        fprintf(stderr, "       Exiting.\n");
        exit(-1);
    }
    else if(errorNo == -2) // -2 means the pointer is in the tree BUT this memory has already been freed
    {
        fprintf(stderr, "Error: memcheckSafe identified a faulty memory access.\n");
        fprintf(stderr, "       Faulty call was on an already freed pointer with address %p\n", (void*)ptr);
        fprintf(stderr, "       Exiting.\n");
        exit(-1);
    }
    else if(errorNo > 0) // Means this pointer IS in the tree, but the sized used exceeded the block
    {
        fprintf(stderr, "Error: memcheckSafe identified a faulty memory access.\n");
        fprintf(stderr, "       Faulty call was on a pointer with address %p and exceeded the allocated block.\n", (void*)ptr);
        fprintf(stderr, "       Attempted to access memory block of size %li when there are only %i bytes available.\n", size, errorNo);
        fprintf(stderr, "       Exiting.\n");
        exit(-1);
    }
}