// This program finds the max. subtree in a binary tree
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

	mainPtr->right->left = newNode(80);
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

int findMaxBST(struct treeStruct *ptr, int& max)
{
	if(ptr == NULL)
		return 0;
    if(ptr->left == NULL && ptr->right == NULL)
        return 1;

	int lc = findMaxBST(ptr->left, max);
	int rc = findMaxBST(ptr->right, max);

    if(ptr->left->element < ptr->element && ptr->right->element > ptr->element && lc != -1 && rc != -1)    
    { 
        if(max<lc+rc+1)
            max = lc+rc+1;
        return lc+rc+1;
    }
    else
        return -1;
    
}

int main()
{
    int mx = 0;
	struct treeStruct *rootPtr = treeCreator();
    findMaxBST(rootPtr, mx);
    cout<<"Size of max binary tree is :"<<mx<<endl;
	return 0;
}



