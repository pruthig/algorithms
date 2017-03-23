//Find maximum element in Binary tree not necessarily BST
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

int counter = 0;

void max(struct treeStruct *ptr, int *m){
if(ptr == NULL)

	return;
max(ptr->left, m);
if(ptr->element > *m)
	*m = ptr->element;

max(ptr->right, m);
}

int main(){
struct treeStruct *rootPtr = treeCreator();
int count = 0;
max(rootPtr, &count);
cout<<"Max node in tree : "<<count<<endl;
return 0;
}



