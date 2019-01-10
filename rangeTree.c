#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "rangeTree.h"

node * createNode(void * ptr, size_t size)
{
    //allocate space for this node
    node * nodeStruct = malloc(sizeof(node));
    if(!nodeStruct)
    {
        fprintf(stderr, "Error: Memory allocation failed. Exiting.");
        exit(EXIT_FAILURE);
    }
    
    
    //structure variables for this interval graph node
    nodeStruct->addrRange = createRange(ptr, size); //pointer to the address range that this block covers
    nodeStruct->max = (void*)(nodeStruct->addrRange->start + nodeStruct->addrRange->end);   //largest value within the subtree of this node
    nodeStruct->left = nodeStruct->right = NULL;    //empty child nodes
    nodeStruct->freed = 0;                          //this memory block (trivially) hasn't been freed yet
    nodeStruct->height = 1;
    
    return nodeStruct;
}



range * createRange(void * ptr, size_t size)
{
    range * rangeStruct = malloc(sizeof(range));
    if(!rangeStruct)
    {
        fprintf(stderr, "Error: Memory allocation failed. Exiting.");
        exit(EXIT_FAILURE);
    }
    
    //structure variables for this interval
    rangeStruct->start = ptr;
    rangeStruct->end = size;
    
    return rangeStruct;
}


//inserts nodes into a list-style tree for deletion from tree purposes only
node * insertNodeList(node ** head, node * newNode)
{
    node *last = *head;
    
    if(*head == NULL)
    {
        *head = newNode;
        return newNode;
    }
    else
    {
        while(last -> right != NULL) //while we are not at the final node
        {
            last = last->right;
        }
        
        last->right = newNode;
        //printf("Adding overlapping node to list!\n");
        return newNode;
    }

}


// function that rotates the tree to the right if unbalanced
node * rotateTreeRight(node * nodeStruct)
{
    node * leftSub = nodeStruct->left; //take left subtree of parameter node
    node * leftRightSub = leftSub->right; //take right subtree of the left parameter subtree

    // rotate right
    leftSub->right = nodeStruct;
    nodeStruct->left = leftRightSub;
    
    // update the current heights
    nodeStruct->height = maxHeight(getHeight(nodeStruct->left), getHeight(nodeStruct->right))+1;
    leftSub->height = maxHeight(getHeight(leftSub->left), getHeight(leftSub->right))+1;
    
    //leftSub->max = nodeStruct->max;
    //nodeStruct->max = (void*)(nodeStruct->addrRange->start + nodeStruct->addrRange->end); //might need to max between this and whatever it had on the right
    return leftSub;
}

// function that rotates the tree to the left if unbalanced
node * rotateTreeLeft(node * nodeStruct)
{
    node * rightSub = nodeStruct->right; //take right subtree of parameter node
    node * rightLeftSub = rightSub->left; //take left subtree of the right paramter subtree
    
    // rotate left
    rightSub->left = nodeStruct;
    nodeStruct->right = rightLeftSub;
    
    // update the current heights
    nodeStruct->height = maxHeight(getHeight(nodeStruct->left), getHeight(nodeStruct->right))+1;
    rightSub->height = maxHeight(getHeight(rightSub->left), getHeight(rightSub->right))+1;
    
    //rightSub->max = nodeStruct->max;
    //nodeStruct->max = (void*)(nodeStruct->addrRange->start + nodeStruct->addrRange->end); 
    return rightSub;
}

// function that checks if the height is more than one for the left/right subtree
int checkBalance(node * current)
{
    if(current == NULL)
    {
        return 0;
    }
    return getHeight(current->left) - getHeight(current->right);
}

node * insertNode(node * root, void* ptr, size_t size)
{
    //BASE CASE | get to end of tree
    if(!root)
    {
        return createNode(ptr, size);
    }
    
    //RECURSIVE CASE | insert node at left or right of root
    //if the our new start addr is lower than the root's start addr, our new node should go to the left subtree
    if(ptr < root->addrRange->start) //if addr is earlier in memory
    {
        root->left = insertNode(root->left, ptr, size); //insert new node at the root's left
    }
    else //if addr is >= in memory
    {
        root->right = insertNode(root->right, ptr, size); //insert new node at the root's right
    }    
    
    // Update height
    root->height = 1 + (maxHeight(getHeight(root->left),getHeight(root->right)));
    
    // Check if tree is still balanced
    int balanced = checkBalance(root);
    
    // 4 cases if tree becomes unbalanced due to current node:
    // left-left case
    // right-right case
    // left-right case
    // right-left case
    
    // left-left
    if(balanced > 1 && ptr < root->left->addrRange->start)
    {
        return rotateTreeRight(root);
    }
    
    //right-right
    if(balanced < -1 && ptr > root->right->addrRange->start)
    {
        return rotateTreeLeft(root);
    }
    
    // left-right
    if(balanced > 1 && ptr > root->left->addrRange->start)
    {
        root->left = rotateTreeLeft(root->left);
        return rotateTreeRight(root);
    }
    
    // right-left
    if(balanced < -1 && ptr < root->right->addrRange->start)
    {
        root->right = rotateTreeRight(root->right);
        return rotateTreeLeft(root);
    }
    
    // return unchanged root pointer if tree still balanced
    return root;
}



node * removeNode(node * root, void * ptr)
{
    void * newMax = NULL; //will hold the new max value if the old max is deleted here!
    void * oldMax = NULL; //will hold the max value corresponding to the node being removed 
                          //this makes it so that we can check later if this was the max and gets removed 
    if(root == NULL)
    {
        return root;
    }
    
    // if ptr is smaller than root pointer, go down left subtree
    if(ptr < root->addrRange->start)
    {
        root->left = removeNode(root->left, ptr);
    }
    // if ptr is larger than root pointer, go down right subtree
    else if(ptr > root->addrRange->start)
    {
        root->right = removeNode(root->right, ptr);
    }
    // ptr matches root pointer
    else
    {
        oldMax = (void*)(root->addrRange->start + root->addrRange->end);
        
        // node with one/zero child
        if((root->left == NULL) || (root->right == NULL))
        {
            node *tempNode = NULL;
            
            // check to see if subtrees are null
            if(root->left != NULL)
            {
                tempNode = root->left;
                newMax = root->left->max;
            }
            if(root->right != NULL)
            {
                tempNode = root->right;
                newMax = root->right->max;
            }
            
            // no child
            if(tempNode == NULL)
            {
                //tempNode = root;
                root = NULL;
                //new max remains NULL
            }
            // one child
            else
            {
                *root = *tempNode;
                //new max is the max of the left or right subtree of this node (whichever it has)
            }
            
            //free(tempNode->addrRange);
            //free(tempNode);
            //tempNode = NULL;
        }
        else
        {
            // two children -> get smallest node in right subtree
            node * tempNode = minNode(root->right);
            
            //save off the original max
            newMax = root->right->max; //should be the same max as before!
            
            // min right subtree node becomes new root
            root->addrRange->start = tempNode->addrRange->start;
            root->right = removeNode(root->right, tempNode->addrRange->start);
        }
    }
    
    // If only one node in tree, return root
    if(root == NULL)
    {
        return root;
    }
    
    // update height of node
    if(root->left || root->right )
    {
        root->height = 1 + maxHeight(getHeight(root->left), getHeight(root->right));
    }
    
    
    //update the max of any ancestor to reflect that a node was removed
    if(newMax != NULL && ((void*)root->max < newMax || (void*)root->max == oldMax))
    {
        root->max = (void*)(newMax);
    }
    
    // check if tree is balanced
    int balanced = checkBalance(root);
    
    // 4 cases if tree becomes unbalanced due to current node:
    // left-left case
    // right-right case
    // left-right case
    // right-left case
    
    // left-left 
    if (balanced > 1 && checkBalance(root->left) >= 0) 
    {
        return rotateTreeRight(root); 
    }
    
    // left-right 
    if (balanced > 1 && checkBalance(root->left) < 0) 
    { 
        root->left =  rotateTreeLeft(root->left); 
        return rotateTreeRight(root); 
    } 
    
    // right-right 
    if (balanced < -1 && checkBalance(root->right) <= 0) 
    {
        return rotateTreeLeft(root); 
    }
    

    // right-left 
    if (balanced < -1 && checkBalance(root->right) > 0) 
    { 
        root->right = rotateTreeRight(root->right); 
        return rotateTreeLeft(root); 
    } 
    
    return root;
}



// Function that finds the minimum node in the right subtree for deletion
node * minNode(node * rightRoot)
{
    node * currentNode = rightRoot;
    
    // find leftmost (smallest) value of the original right subtree
    while(currentNode->left != NULL)
    {
        currentNode = currentNode->left;
    }
    
    return currentNode;
}




// function that gets the height at the current node
int getHeight(node * current)
{
    if(current == NULL)
    {
        return 0;
    }
    return current->height;
}

// function that compares the height of left and right subtree
int maxHeight(int height1, int height2)
{
    if(height1 > height2)
    {
        return height1;
    }
    else
    {
        return height2;
    }
}

//checks if this exact pointer is in the tree already
//useful for
//    - checking if a pointer is already freed
//    - reallocing an existing pointer
//    - freeing a pointer 
node * checkTreeContainsPtr(node * root, void * searchKey, int freeFlag)
{
    while(root)
    {
        //check if this is the right node
        if(root->addrRange->start == searchKey)
        {
            //check if trying to free already free block
            if(root->freed == 1)
            {
                if(freeFlag == 1) //double free
                {
                    //double free : return error
                    fprintf(stderr, "Error: Call to freeSafe results in a double free.\n");
                    fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)searchKey);
                    fprintf(stderr, "       Exiting.\n");
                    
                    exit(-1);
                }
                if(freeFlag == 2) //double free from realloc
                {
                    //double free : return error
                    fprintf(stderr, "Error: Call to reallocSafe attempts to reallocate an already freed block.\n");
                    fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)searchKey);
                    fprintf(stderr, "       Exiting.\n");
                    
                    exit(-1);
                }
                else //this pointer is in the tree BUT it has already been freed - just fine!
                {
                    return NULL;
                }
            }
            

            //testing print
            //printf("Search function found matching pointer at height %i: %p\n", height, (void*)root->addrRange->start);
            //end test print
            
            return root;
        }
        else if(searchKey >= root->addrRange->start && searchKey <= root->addrRange->start + root->addrRange->end)
        {
            if(freeFlag == 1)
            {
                //attempting to free using an address that isn't the start of the memory block
                fprintf(stderr, "Error: Call to freeSafe is not made on the first byte of the allocated memory range.\n");
                fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)searchKey);
                fprintf(stderr, "       This pointer is located inside the memory block of size %li starting at address %p\n", 
                        root->addrRange->end, (void*)root->addrRange->start);
                fprintf(stderr, "       Exiting.\n");
                exit(-1);
            }
            else if(freeFlag == 2) //from realloc
            {
                //attempting to reallocate using an address that isn't the start of the memory block
                fprintf(stderr, "Error: Call to reallocSafe is not made on the first byte of the allocated memory range.\n");
                fprintf(stderr, "       Faulty call was on pointer with address %p\n", (void*)searchKey);
                fprintf(stderr, "       This pointer is located inside the memory block of size %li starting at address %p\n", 
                        root->addrRange->end, (void*)root->addrRange->start);
                fprintf(stderr, "       Exiting.\n");
                exit(-1);
            }
            
            
        }
        else
        {
            if(searchKey < root->addrRange->start)
            {
                root = root->left;
            }
            else //>=
            {
                root = root->right;
            }
            
        }
    }
    
    //if you get to the end of the tree and you didn't find it, it isn't here
    return NULL;
}


//Checks if the query interval is contained entirely within one node's interval in the range tree
// RETURN CODE KEY:
//      0 --> interval found - no problem
//      -1 --> searchKey not in tree at all
//      + --> searchKey in the tree, but size TOO big 
//      -2 --> search key found but memory already freed    
int checkTreeContainsInterval(node * root, void * searchKey, size_t size)
{
    int lowerBoundFound = 0;
    while(root)
    {
        //check if this node and the searchKey pointer+size overlap
        if(searchKey >= root->addrRange->start && searchKey + size <= root->addrRange->start + root->addrRange->end)
        {
            //the node is contained in this interval!
            if(root->freed == 1)
            {
                return -2;
            }
            else
            {
                return 0;
            }
        }
        else
        { 
            //check if the node would otherwise fit into this interval but the size is too large
            if(searchKey >= root->addrRange->start && searchKey <= root->addrRange->start + root->addrRange->end)
            {
                lowerBoundFound = root->addrRange->end;
            }
            
            
            //go to the left or right subtree
            if(searchKey < root->addrRange->start) //lower - left
            {
                root = root->left;
            }
            else
            {
                root = root->right; //higher - right 
            }
        }    
    }
    
    //if you got here, the interval doesn't fit in the tree - must return correct error code
    if(lowerBoundFound > 0)
    {
        return lowerBoundFound;
    }
    else
    {
        return -1;
    }
}


//When malloc or realloc gets a pointer assigned (and an accompanying size) we have to update the tree with this new node
//BUT before we can insert the new node (will happen right after this call) we need to remove any node that would overlap with this one
//So we add all overlapping nodes to a list so they can be deleted!
int checkTreeBlockBounds(node * root, void * searchKey, size_t size, node ** nodesToDelete)
{    
    if(root == NULL) //got to the end!
    {
        return 0;
    }
    
    
    if(root->addrRange->start <= (void*)(searchKey + size) && (void*)(root->addrRange->start + root->addrRange->end) >= searchKey)
    {        
        //if this node is the START do the start checks
        if(root->addrRange->start == searchKey) //if this node is at the start of the old freed node
        {
            //this node should just be deleted
        }
        
        //if this interval starts somewhere in the middle of the old freed node
        else if(root->addrRange->start < searchKey && (void*)(root->addrRange->start + root->addrRange->end) >= searchKey)
        {
            //update old node to being a free block going from the original start to the start of our new block
            root->addrRange->end = (searchKey - root->addrRange->start);
        }
        
        //otherwise, add the whole node to the delete list 
        *nodesToDelete = insertNodeList(nodesToDelete, root); //add this node to the nodesToDeleteList

    }
    
    //pick the lower or higher intervals for the next recursive iteration 
    if(searchKey < root->addrRange->start) //lower
    {
        checkTreeBlockBounds(root->left, searchKey, size, nodesToDelete);
        
    }
    else //higher
    {
        checkTreeBlockBounds(root->right, searchKey, size, nodesToDelete);
    }
    return 0;
}
