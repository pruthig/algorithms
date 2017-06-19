// This tree creates the copy of a given tree using recursion
#include<iostream>

using namespace std;


typedef struct treeStruct
{
	int element;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
	struct treeStruct *newElement = new(struct treeStruct);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->element = data;
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

treeStruct* getNewTree(treeStruct *oldPtr) {
    treeStruct *newPtr = NULL;
    if(oldPtr) 
        newPtr = newNode(oldPtr->element);
    else
        return NULL;

    newPtr->left = getNewTree(oldPtr->left);
    newPtr->right = getNewTree(oldPtr->right);
    return newPtr;

}

void traverseNodes(struct treeStruct *ptr)
{
	if(ptr == NULL)
		return;
	traverseNodes(ptr->left);
	cout<<ptr->element<<"   ";
	traverseNodes(ptr->right);
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
    treeStruct *newRootPtr = getNewTree(rootPtr);
    // Now, traverse the new tree
	traverseNodes(newRootPtr);
    cout<<endl;
	return 0;
}



