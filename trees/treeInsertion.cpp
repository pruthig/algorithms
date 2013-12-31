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

void printSortedTree(struct treeStruct *ptr){
if(ptr == NULL)
	return;

printSortedTree(ptr->left);
cout<<ptr->element<<",";
printSortedTree(ptr->right);

}

struct treeStruct* treeCreator(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = newNode(12);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(10);

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);


return mainPtr;
}

struct treeStruct* insertNode(struct treeStruct *ptr, int element){
if(ptr == NULL)
{
	return newNode(element);
}
if(element < ptr->element)
	ptr->left  = insertNode(ptr->left, element);
else
	ptr->right  = insertNode(ptr->right, element);
return ptr;
}

int main(){
int new_node = 0;
struct treeStruct *rootPtr = treeCreator();
cout<<"Before insertion inorder traversal of tree is :"<<endl;
printSortedTree(rootPtr);

cout<<"Enter the node number you wanna insert"<<endl;
cin>>new_node;
insertNode(rootPtr, new_node);
cout<<"After insertion inorder traversal of tree is :"<<endl;
printSortedTree(rootPtr);
return 0;
}



