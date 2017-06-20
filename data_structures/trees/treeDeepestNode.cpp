// Find deepest node of tree
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

int getDeepestNode(struct treeStruct *ptr, int level, int maxLevel)
{
    if(ptr == NULL)
        return 0;
	if(ptr->left == NULL && ptr->right == NULL) {
        if(level > maxLevel) {
            maxLevel = level;
            return ptr->element;
        }
    }
	getDeepestNode(ptr->left, level+1, maxLevel);
	getDeepestNode(ptr->right, level+1, maxLevel);
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	cout<<"Deepest node of tree is : "<<getDeepestNode(rootPtr, 0, 0)<<endl;
	return 0;
}



