//We will define a "root-to-leaf path" to be a sequence of nodes in a tree starting with the root node and proceeding downward to a leaf 
//(a node with no children). We'll say that an empty tree contains no root-to-leaf paths.
//we will be concerned with the sum of the values of such a path 

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

mainPtr->right->right->right->left = newNode(5);
mainPtr->right->right->right->right = newNode(30);

return mainPtr;
}

bool found = false;

bool hasSumPath(struct treeStruct *ptr, int sum, int counter){
if(ptr != NULL)
{
	counter +=ptr->element;
}

if(ptr == NULL){
	if(sum == counter)
	{
		found = true;
		return true;
	}
	else
		return false;
}

hasSumPath(ptr->left, sum, counter);
hasSumPath(ptr->right, sum, counter);
return false;
}

int main(){
int sum = 0;
struct treeStruct *rootPtr = treeCreator();
cout<<"Enter the sum you want to check"<<endl;
cin>>sum;
hasSumPath(rootPtr, sum, 0);

if(found == false)
cout<<"Not found the match .."<<endl;
else
cout<<"Match found"<<endl;

return 0;
}



