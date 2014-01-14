//this program finds the lowest common ancestor in a tree

#include<iostream>

using namespace std;

void rootToLeafPrinter(struct treeStruct *ptr);
struct treeStruct* newNode(int data);
struct treeStruct* treeCreator();

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

mainPtr->right->left = newNode(11	);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(15);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int findLCA(struct treeStruct *ptr, int s1, int s2)
{
if(ptr == NULL)
	return 0;
else if(s1 == ptr->element || s2 == ptr->element)
	return ptr->element;
else if(s1 < ptr->element && s2 > ptr->element)
	return ptr->element;
else if(s1 < ptr->element && s2 < ptr->element)
	findLCA(ptr->left, s1, s2);
else
	findLCA(ptr->right, s1, s2);

}

	
int main(){
int s1, s2;
struct treeStruct *rootPtr = treeCreator();
cout<<"Enter the 2 nodes\n";
cin>>s1>>s2;
int ans = findLCA(rootPtr, s1, s2);
cout<<"Lowest common ancestor is :"<<ans<<endl;
return 0;
}


void rootToLeafPrinter(struct treeStruct *ptr){
if(ptr == NULL){
	return;
}

rootToLeafPrinter(ptr->left);  //6 ka left completed
cout<<ptr->element<<" ";
rootToLeafPrinter(ptr->right);
return;
}


