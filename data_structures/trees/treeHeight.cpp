// Find tree height...
#include<iostream>
#include<algorithm>


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
    
	mainPtr->right->right->right->right = newNode(24);

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
                                                  \
                                                  24
*/

int getHeight(struct treeStruct *ptr)
{
	if(ptr == NULL)
		return 0;
	int l = getHeight(ptr->left);
	int r = getHeight(ptr->right);
    return max(l, r)+1;
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	cout<<"Height of tree is : "<<getHeight(rootPtr)<<endl;
	return 0;
}



