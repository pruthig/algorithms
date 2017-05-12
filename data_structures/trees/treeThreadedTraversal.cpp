// This program creates a threaded tree from a normal tree and traverse it without recursion
// In which wherever we have right NULL pointer we point it to the next successor. Also, we mark that node has threaded
#include<iostream>


using namespace std;

typedef struct treeStruct
{
	int element;
	struct treeStruct *left;
	struct treeStruct *right;
    bool isThreaded;
}treeStruct;

struct treeStruct *prev = NULL;

struct treeStruct* newNode(int data)
{
	struct treeStruct *newElement = new(struct treeStruct);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->element = data;
    newElement->isThreaded = false;
	return newElement;
}

struct treeStruct* treeCreator()
{
	//create the main pointer
	struct treeStruct *mainPtr;
	mainPtr = newNode(10);
	mainPtr->left = newNode(6);
	mainPtr->right = newNode(12);

	mainPtr->left->left = newNode(2);
	mainPtr->left->right = newNode(9);

	mainPtr->right->left = newNode(11);
	mainPtr->right->right = newNode(18);

	mainPtr->right->right->left = newNode(16);
	mainPtr->right->right->right = newNode(20);

	return mainPtr;
}
/*
                                        10
                                      /    \
                                    6      12
                                   / \     / \
                                  2   9   11  18
                                              / \
                                            16   20
*/

void traverseNodes(struct treeStruct *ptr)
{
	if(ptr == NULL)
        return;
    while(ptr->left != NULL) {
        ptr = ptr->left;
    }
    while(ptr){

        cout<<ptr->element<<", ";
        if(!ptr->isThreaded) {
            for(ptr = ptr->right; ptr != NULL && ptr->left != NULL; ptr = ptr->left);
        }
        else 
            ptr = ptr->right;
    }
}

void convertToThreaded(struct treeStruct *ptr) {
    if(ptr == NULL)
        return;
    if(ptr->left == NULL && ptr->right == NULL) {
        prev = ptr;
        return;
    }
       
    convertToThreaded(ptr->left);
    if(prev) { 
        prev->right = ptr;
        prev->isThreaded = true;
    }
     
    convertToThreaded(ptr->right);
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
    convertToThreaded(rootPtr);
	traverseNodes(rootPtr);
    cout<<endl;
	return 0;
}



