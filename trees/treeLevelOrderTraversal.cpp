//program prints out the elements at given level using simple recursion
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

mainPtr->right->right->left = newNode(15);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int counter = 0;

void printNodes(struct treeStruct *ptr, int n){
if(ptr == NULL)
	return;
else if( n==1 ){
	cout<<ptr->element<<", "; 
	return;
}
else{
printNodes(ptr->left, n-1);
printNodes(ptr->right, n-1);
}

}

int main(){
int lvl = 0;
struct treeStruct *rootPtr = treeCreator();
cout<<"Nothing will be printed if level is greater than the height";
cout<<"enter the level number\n";
cin>>lvl;
printNodes(rootPtr, lvl);
return 0;
}



