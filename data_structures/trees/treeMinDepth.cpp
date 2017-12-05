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

struct treeStruct* treeCreator(){
	//create the main pointer
	struct treeStruct *mainPtr;
	mainPtr = newNode(10);
	mainPtr->left = newNode(6);
	mainPtr->right = newNode(12);
	
	mainPtr->left->left = newNode(2);
	mainPtr->left->right = newNode(10);
	
	mainPtr->right->left = newNode(4);
	mainPtr->right->right = newNode(18);
	
	mainPtr->right->right->left = newNode(0);
	mainPtr->right->right->right = newNode(20);
	
	mainPtr->right->right->right->left = newNode(5);
	mainPtr->right->right->right->right = newNode(30);
	
	return mainPtr;
}

void treeMinDepth(struct treeStruct *ptr, int counter, int& min_depth){
	if(ptr == NULL)
		return;
	
	if(ptr->left == nullptr && ptr->right == nullptr) {
		if(counter < min_depth)
	  		min_depth = counter;	
		return;
	}	
	treeMinDepth(ptr->left, counter+1, min_depth);
	treeMinDepth(ptr->right, counter+1, min_depth);
}

int main(){
	struct treeStruct *rootPtr = treeCreator();
	int minDepth = INT_MAX;
	treeMinDepth(rootPtr, 0, minDepth);
	cout<<"Min Depth of tree : "<<minDepth<<endl;
	return 0;
}



