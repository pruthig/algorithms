// Given a binary tree containing digits from 0-9 only, each root-to-leaf path could represent a number.
// An example is the root-to-leaf path 1->2->3 which represents the number 123.
// Find the total sum of all root-to-leaf numbers % 1003. 

#include<iostream>


using namespace std;

typedef struct treeStruct
{
	int data;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
	struct treeStruct *newElement = new(struct treeStruct);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->data = data;
	return newElement;
}

struct treeStruct* treeCreator()
{
	//create the main pointer
	struct treeStruct *mainPtr;
	mainPtr = newNode(1);
	mainPtr->left = newNode(6);
	mainPtr->right = newNode(2);

	mainPtr->left->left = newNode(2);
	mainPtr->left->right = newNode(9);

	mainPtr->right->left = newNode(4);
	mainPtr->right->right = newNode(8);

	mainPtr->right->right->left = newNode(6);
	mainPtr->right->right->right = newNode(2);

	return mainPtr;
}
/*
                                        1
                                      /    \
                                    6      2
                                   / \     / \
                                  2   9   4   8
                                             / \
                                            6   2
*/

void findSumNumbers(struct treeStruct *ptr, int count, int& sum)
{
	if(!ptr->left && !ptr->right) {
        count = count*10 + ptr->data;
        sum += count;
		return;
    }
	findSumNumbers(ptr->left, (count*10 + ptr->data), sum);
	findSumNumbers(ptr->right, (count*10 + ptr->data), sum);
}

int main()
{
    int sum = 0;
	struct treeStruct *rootPtr = treeCreator();
	findSumNumbers(rootPtr, 0, sum);
    cout<<"Final sum is: "<<sum<<endl;
	return 0;
}



