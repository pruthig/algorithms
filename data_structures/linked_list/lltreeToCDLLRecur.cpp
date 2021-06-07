//create DLL from tree

#include<iostream>

using std::cout;
using std::cin;
using std::endl;

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

struct treeStruct* head = NULL;
struct treeStruct* prev = NULL;
struct treeStruct* tail = NULL;

void treeToDLL(struct treeStruct *cur){

if(cur == NULL)
	return;	

treeToDLL(cur->left);
if(head == NULL) {
  head = cur;
  prev = cur;
}
else{
	cur->left = prev;
	prev->right = cur;
	prev = cur;
}
	
treeToDLL(cur->right);
// Adjust for tail
if (cur != NULL && cur->right == NULL)
  tail = cur;
}

int main(){
struct treeStruct *rootPtr = treeCreator();
treeToDLL(rootPtr);
print(head);
return 0;
}

void print(struct treeStruct* head){
struct treeStruct* temp = head;
int c;
while(temp->right != head){
	cout<<temp->element<<", ";
	temp = temp->right;
  cin>>c;
}

cout<<endl;
}



