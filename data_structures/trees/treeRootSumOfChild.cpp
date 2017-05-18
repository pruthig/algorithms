// This program will find if there exists a subtree with a given sum 
// in the present tree..
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

int findSumSubtree(struct treeStruct *ptr, int sum)
{
	if(ptr == NULL)
		return 0;
	int lsum = findSumSubtree(ptr->left, sum);
	int rsum = findSumSubtree(ptr->right, sum);
    if(lsum == INT_MAX || rsum == INT_MAX)
        return INT_MAX;

    int temp_sum  = (lsum+rsum+ptr->element);
    if(temp_sum == sum) {
        return INT_MAX;
    }
    return temp_sum;
}

int main()
{
    int sum = 0;
	struct treeStruct *rootPtr = treeCreator();
    cout<<"Enter the sum\n";
    cin>>sum;
	int s = findSumSubtree(rootPtr, sum);
    if(s == INT_MAX)
        cout<<"Subtree found\n";
    else
        cout<<"Subtree not found\n";

	return 0;
}



