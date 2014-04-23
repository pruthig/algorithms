//This program checks if two trees are foldable
//two trees are foldable if both are mirror image of each other
//irrespective of the data in the node 

#include<iostream>
	

using namespace std;

bool checkIfFoldable(struct treeStruct *ptr1, struct treeStruct *ptr2);


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

struct treeStruct* treeCreator1(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = NULL; //newNode(12);

return mainPtr;
}

struct treeStruct* treeCreator2(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = NULL;

return mainPtr;
}

bool checkIfFoldable(struct treeStruct *ptr1, struct treeStruct *ptr2)
{
		if( ptr1 == NULL && ptr2 == NULL)
			return true;
		return ( ( (ptr1 && ptr2) || (ptr1 == NULL && ptr2 == NULL) ) && checkIfFoldable(ptr1->left, ptr2->right) && checkIfFoldable(ptr1->right, ptr2->left));
}

int main(){
struct treeStruct *rootPtr1 = treeCreator1();
struct treeStruct *rootPtr2 = treeCreator2();

bool isEqual = checkIfFoldable(rootPtr1, rootPtr2);
cout<<"Trees are equal : "<<isEqual<<endl;
return 0;
}



