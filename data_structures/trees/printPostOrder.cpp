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
mainPtr = newNode(4);
mainPtr->left = newNode(2);
mainPtr->right = newNode(5);

mainPtr->left->left = newNode(1);
mainPtr->left->right = newNode(3);

return mainPtr;
}

void printPostOrder(struct treeStruct *ptr){
if(ptr == NULL)
	return;

printPostOrder(ptr->left);
printPostOrder(ptr->right);
cout<<ptr->element<<",";

}

int main(){
struct treeStruct *rootPtr = treeCreator();
int counter = 0;
printPostOrder(rootPtr);
return 0;
}



