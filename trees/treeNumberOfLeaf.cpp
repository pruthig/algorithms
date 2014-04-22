//this program finds the number of leaves in a binary tree
//Simple logic is to go for inorder traversal... and for nodes whose left and right are 
//LEAF nodes

#include<iostream>
#include<cmath>


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
mainPtr = newNode(8);
mainPtr->left = newNode(6);
mainPtr->right = newNode(2);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(1);
mainPtr->right->right = newNode(8);

mainPtr->right->right->left = newNode(5);
mainPtr->right->right->right = newNode(0);

return mainPtr;
}

int c = 0;

void countLeafNodes(struct treeStruct *ptr){
if(ptr->left == NULL && ptr->right == NULL)
{
	++c; 
	return;
}
	
countLeafNodes(ptr->left);
countLeafNodes(ptr->right);
}

int main(){
struct treeStruct *rootPtr = treeCreator();
//int count = 
countLeafNodes(rootPtr);
cout<<"Number of leaf nodes in tree : "<<c<<endl;
return 0;
}



