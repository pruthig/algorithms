//compact way


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

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(16);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

bool traverse(struct treeStruct *ptr, int node){

if(ptr == NULL)
	return false;

if(node == ptr->element)
	return true;

if( traverse(ptr->left,  node) || traverse(ptr->right, node)){
	cout<<ptr->element<<", ";
	return true;
}
return false;

}


int main(){
int node = 0;
cout<<"enter the input\n";
cin>>node;
struct treeStruct *rootPtr = treeCreator();
bool y  =traverse(rootPtr, node);
cout<<endl;
return 0;
}


