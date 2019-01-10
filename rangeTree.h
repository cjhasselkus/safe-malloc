#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

typedef struct range 
{ 
    void* start;   //address of corresponding ptr
    size_t end;   //where this memory block ends (ptr address + size)
   
} range; 

typedef struct node 
{ 
    range * addrRange;     //pointer to the address range that this block covers
    void * max;            //largest value within the subtree of this node -  was used previously but ended up being unnecessary 
    struct node * left;    //left node (lower address interval)
    struct node * right;   //right node (higher address interval)
    int freed;             //indicates if this memory block has been freed already and shouldn't be re-freed
    int height;
   
} node; 


node * createNode(void * ptr, size_t size);
range * createRange(void * ptr, size_t size);
node * insertNode(node * root, void* ptr, size_t size);
node * insertNodeList(node ** head, node * newNode);
node * removeNode(node * root, void* ptr);
node * minNode(node * rightNode);
int checkBalance(node * current);
int maxHeight(int height1, int height2);
int getHeight(node * current);
void preOrder(node * root);
node * checkTreeContainsPtr(node * root, void * searchKey, int freeFlag);  //for free and malloc
int checkTreeContainsInterval(node * root, void * searchKey, size_t size); //most useful for memcheck
int checkTreeBlockBounds(node * root, void * searchKey, size_t size, node ** nodesToDelete);      //for malloc and realloc

#endif // LINKEDLIST_H_