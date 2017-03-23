//This program creates a copy of tree using recursion..
//logic >> create parent node then respective left and right calls for creation 
//Initially pass the address of the root node of the new tree to reflect the changes in the tree

#include<iostream>


using namespace std;

void rootToLeafPrinter(struct treeStruct *ptr);
struct treeStruct* newNode(int data);
struct treeStruct* treeCreator();

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

return mainPtr;
}

void mirrorTree(struct treeStruct *ptr1, struct treeStruct **ptr2)
{
	if(ptr1 != NULL)
	{
		*ptr2 = newNode(ptr1->element);
		mirrorTree(ptr1->left, &((*ptr2)->left));
		mirrorTree(ptr1->right, &((*ptr2)->right));
	}
}
	
int main(){
struct treeStruct *rootPtr = treeCreator();
struct treeStruct *mirrorPtr = NULL;
mirrorTree(rootPtr, &mirrorPtr);
cout<<"printing the mirror tree in inorder fashion "<<endl;
rootToLeafPrinter(mirrorPtr);
return 0;
}


void rootToLeafPrinter(struct treeStruct *ptr){
if(ptr == NULL){
	return;
}

cout<<ptr->element<<" ";

rootToLeafPrinter(ptr->left);  //6 ka left completed
rootToLeafPrinter(ptr->right);
return;
}


