// This tree calculates the minimum distance between 2 nodes.

#include<iostream>
#include<climits>


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
	mainPtr->left->right = newNode(-9);

	mainPtr->right->left = newNode(1);
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
                                  2  -9   1  18
                                              / \
                                            16   20
*/

int calculateMin(struct treeStruct *ptr, int *maxdiff)
{
	if(ptr->left == NULL && ptr->right == NULL) {
		return ptr->element;
    }

	int leftmin = calculateMin(ptr->left, maxdiff);
	int rightmin = calculateMin(ptr->right, maxdiff);

    int calmin = leftmin<rightmin?leftmin:rightmin;
    if(ptr->element - calmin > *maxdiff) {
        *maxdiff = ptr->element - calmin;
    }
    if(ptr->element < calmin)
        return ptr->element;
    else 
        return calmin;
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
    int min = INT_MAX, maxdiff = INT_MIN;
	calculateMin(rootPtr, &maxdiff);
    cout<<"\nThe max difference between an ancestor and child is : "<<maxdiff<<endl;
	return 0;
}



