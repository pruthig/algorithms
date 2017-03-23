//this program check if the given tree is BST

#include<iostream>
#include<cmath>


using namespace std;

bool isBST(struct treeStruct *);

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
mainPtr = newNode(8);
mainPtr->left = newNode(6);
mainPtr->right = newNode(12);

mainPtr->left->left = newNode(9);
mainPtr->left->right = newNode(5);

mainPtr->right->left = newNode(10);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(16);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}


bool isBST(struct treeStruct *ptr){
if(ptr->left == NULL && ptr->right == NULL ) 
	return true;

return( isBST(ptr->left) && isBST(ptr->right) && (ptr->left->element < ptr->element)&& (ptr->right->element > ptr->element));

}

int main(){
struct treeStruct *rootPtr = treeCreator();
cout<<"s tree BST : "<<isBST(rootPtr)<<endl;
return 0;
}



