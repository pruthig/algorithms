//create DLL from tree

#include<iostream>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;

void print(struct treeStruct* start);

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

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(16);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int counter = 0;

//inorder traversal

struct treeStruct* old = NULL;
struct treeStruct* dll = NULL;
bool t = false;

void  countNodes(struct treeStruct *cur){

if(cur == NULL)
	return;	

countNodes(cur->left);
if(!t){
	dll = cur;
	t = true;
	old = dll;
}

else{
	cur->left = old;
	old->right = cur;
	old = cur;
}
	
countNodes(cur->right);

}

int main(){
struct treeStruct *rootPtr = treeCreator();
countNodes(rootPtr);
//c->right = NULL;
print(dll);
return 0;
}

void print(struct treeStruct* start){
while(start != NULL){
	cout<<start->element<<", ";
	start = start->right;
}

cout<<endl;
}



