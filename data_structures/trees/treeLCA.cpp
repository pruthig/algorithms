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
	
	mainPtr->right->left = newNode(11);
	mainPtr->right->right = newNode(18);
	
	mainPtr->right->right->left = newNode(15);
	mainPtr->right->right->right = newNode(20);
	
	return mainPtr;
}

bool findLCA(struct treeStruct *ptr, int val1, int val2, int& res)
{
	if(ptr == NULL)
		return false;
	if(ptr->element == val1 || ptr->element == val2)
		return true;
	bool bl = findLCA(ptr->left, val1, val2, res) || ptr;
	bool bf = findLCA(ptr->right, val1, val2, res);
	if(bl && bf) 
	{
		if(res == -1 )
			res = ptr->element;
	}
	return (bl||bf);
}

	
int main(){
	int s1, s2;
	struct treeStruct *rootPtr = treeCreator();
	cout<<"Enter the 2 nodes\n";
	cin>>s1>>s2;
	int res = -1;
	findLCA(rootPtr, s1, s2, res);
	cout<<"Lowest common ancestor is :"<<res<<endl;
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


