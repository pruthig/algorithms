//this program prints all the ancestors of given node...
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

/*
                                        10
                                      /    \
                                    6      12
                                   / \     / \
                                  2   9   11  18
                                              / \
                                            16   20
*/


void findNodes(struct treeStruct *ptr, int e){
if(ptr == NULL){
	cout<<"Element not found\n";
	return ;
}
else if(ptr->element == e){
	cout<<"Element found\n";
	return;
}
else if(e < ptr->element){
	cout<<ptr->element<<", ";
	findNodes(ptr->left, e);
}
else{
	findNodes(ptr->right, e);
	cout<<ptr->element<<", ";
}
}

int main(){
struct treeStruct *rootPtr = treeCreator();
int element = 0;
cout<<"Enter the element you want to find\n";
cin>>element;
findNodes(rootPtr, element);
return 0;
}



