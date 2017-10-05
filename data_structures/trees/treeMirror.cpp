//This program create a mirror image of tree by swapping left and right chld in bottom- up fashion..

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

void mirrorTree(struct treeStruct *ptr)
{
	if(ptr == NULL)
		return;
	
	mirrorTree(ptr->left);
	mirrorTree(ptr->right);
		
	struct treeStruct *temp = ptr->left;
	ptr->left = ptr->right;
	ptr->right = temp;
}

	
int main(){
struct treeStruct *rootPtr = treeCreator();
cout<<"Before mirroring in inorder fashion "<<endl;
rootToLeafPrinter(rootPtr);
mirrorTree(rootPtr);
cout<<"After mirroring in inorder fashion "<<endl;
rootToLeafPrinter(rootPtr);
return 0;
}


void rootToLeafPrinter(struct treeStruct *ptr){
	if(ptr == NULL){
		return;
	}
	
	rootToLeafPrinter(ptr->left);  //6 ka left completed
	cout<<ptr->element<<" ";
	rootToLeafPrinter(ptr->right);
	return;
}


