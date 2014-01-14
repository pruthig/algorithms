//This program deletes a given tree
//first it deletes the left and right child and then the give node of the tree

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

return mainPtr;
}

void deleteNodes(struct treeStruct *ptr){
if(ptr == NULL)
	return;
deleteNodes(ptr->left);
deleteNodes(ptr->right);

delete(ptr);
ptr = NULL;
}

int main(){
struct treeStruct *rootPtr = treeCreator();
deleteNodes(rootPtr);
return 0;
}



