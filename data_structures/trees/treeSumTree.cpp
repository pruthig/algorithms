//this program will find is the given binary tree is a Sum tree which 
//where total of the elements of its left and right subtree is equal to root

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
mainPtr = newNode(35);
mainPtr->left = newNode(6);
mainPtr->right = newNode(29);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(4);

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

return mainPtr;
}


int isSumTree(struct treeStruct *ptr, bool* var){
if(ptr->left == NULL && ptr->right == NULL)
	return ptr->element;
if(ptr->element != (isSumTree(ptr->left, var) + isSumTree(ptr->right, var)) )
	*var = false;	
return ptr->element;
}

int main(){
struct treeStruct *rootPtr = treeCreator();
bool var = true;

int count = isSumTree(rootPtr, &var);
cout<<"Is sum..?"<<var<<endl;
return 0;
}



