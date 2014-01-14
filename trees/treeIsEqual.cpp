//This program checks if the 2 trees are equal 

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
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(4);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(13);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int counter = 0;

bool checkIfEqual(struct treeStruct *ptr1, struct treeStruct *ptr2){
if( (ptr1 == NULL && ptr2 != NULL) || ( ptr1 != NULL && ptr2 == NULL))
	return false;
if(ptr1 == NULL && ptr2 == NULL)
	return true;

return ( ptr1->element == ptr2->element && checkIfEqual(ptr1->left, ptr2->left) && checkIfEqual(ptr1->right, ptr2->right));
}

int main(){
struct treeStruct *rootPtr1 = treeCreator();
struct treeStruct *rootPtr2 = treeCreator();

bool isEqual = checkIfEqual(rootPtr1, rootPtr2);
cout<<"Trees are equal : "<<isEqual<<endl;
return 0;
}



