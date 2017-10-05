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

int maxDepth = 0;

int treeMaxDepth(struct treeStruct *ptr, int counter){
	if(ptr != NULL)
		counter++;
	
	if(ptr == NULL){
		if(counter > maxDepth)
	  	maxDepth = counter;	
		return counter;
	}
	
	treeMaxDepth(ptr->left, counter);
	treeMaxDepth(ptr->right, counter);
	
	return maxDepth-1;
}

int main(){
struct treeStruct *rootPtr = treeCreator();
int counter = 0;
int count = treeMaxDepth(rootPtr, counter );
cout<<"Depth of tree : "<<count<<endl;
return 0;
}



