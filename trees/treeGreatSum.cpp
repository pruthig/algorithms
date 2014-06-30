//This program converts a tree in a  great sum tree where
//each node contains the sum of nodes greater than that node
//Can be solved with right-left inorder traversal

#include<iostream>


using namespace std;

typedef struct treeStruct{
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
	mainPtr->left->right = newNode(8);
	
	mainPtr->right->left = newNode(11);
	mainPtr->right->right = newNode(18);
	
	mainPtr->right->right->right = newNode(20);
	
	return mainPtr;
}

void countNodes(struct treeStruct *ptr , int *sum)
{
	if(ptr == NULL)
		return;
	
	countNodes(ptr->right, sum);
	*sum = ptr->element + *sum;
	ptr->element = *sum;
	countNodes(ptr->left, sum);
	
	return;
}

void printTree(struct treeStruct *p)
{
	if(p == NULL)
		return;
	printTree(p->left);
	cout<<p->element<<", ";
	printTree(p->right);
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	cout<<"Original tree "<<endl;
	printTree(rootPtr);
	int count = 0;
	countNodes(rootPtr, &count);
	cout<<"New tree \n";
	printTree(rootPtr);
	return 0;
}


