// This program finds maximum path sum along any path in the tree
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
    

	mainPtr->right->left->left = newNode(50);
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

int findMaxPathSum(struct treeStruct *ptr, int& sum)
{
	if(ptr == NULL)
		return 0;
    if(ptr->left == NULL && ptr->right == NULL)
        return ptr->element;

    int lsum = 	findMaxPathSum(ptr->left, sum);
    int rsum =  findMaxPathSum(ptr->right, sum);
    if(lsum+rsum+ptr->element > sum) {
        sum = lsum + rsum + ptr->element;
    }
    return max( max( lsum, rsum ) + ptr->element, ptr->element );
}

int main()
{
    int sum = 0;
	struct treeStruct *rootPtr = treeCreator();
    findMaxPathSum(rootPtr, sum);
	cout<<"Max path sum in tree is : "<<sum<<endl;
	return 0;
}



