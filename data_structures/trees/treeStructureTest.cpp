//All nodes finder will find all the nodes having right and left childern
#include<iostream>
#include<queue>


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

struct treeStruct* createTree1(){
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

struct treeStruct* createTree2(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = newNode(12);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(9);

return mainPtr;
}




int count = 0;

bool areTreesSimilar(struct treeStruct *ptr1, struct treeStruct *ptr2)
{
	if(ptr1 == NULL && ptr2 == NULL)
		return true;
	else if( (ptr1 == NULL && ptr2 != NULL ) || ( ptr1 != NULL && ptr2 == NULL))
		return false;
	else
	{
		return ( areTreesSimilar(ptr1->left, ptr2->left) && areTreesSimilar(ptr1->right, ptr2->right) );
	}
	
}

int main()
{
	struct treeStruct *ptr1 = createTree1();
	struct treeStruct *ptr2 = createTree2();
	cout<<"Trees Smilarity "<<areTreesSimilar(ptr1, ptr2)<<endl;
	return 0;
}



