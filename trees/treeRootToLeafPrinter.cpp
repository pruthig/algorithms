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

int arr[10000];
int Counter = -1;

void pop(struct treeStruct *tP){
	if(tP == NULL)
		return;
	else{
	arr[Counter] = -1;
	--Counter;
	}
}

void print(){
	int i=0;
	while(1){
		if(arr[i] == -1){
			cout<<endl;
			return;
		}
		else
			cout<<arr[i]<<", ";
		i++;
	}
}

void rootToLeafPrinter(struct treeStruct *ptr){
if(ptr == NULL){
	print();
	return;
}
else
{	Counter++;
	arr[Counter] = ptr->element;
}

if(ptr->left == NULL && ptr->right == NULL){
	print();
	return;
}

rootToLeafPrinter(ptr->left);  //6 ka left completed
pop(ptr->left);
rootToLeafPrinter(ptr->right);
pop(ptr->right);

return;
}

int main(){
std::fill_n(arr, 10000, -1);
struct treeStruct *rootPtr = treeCreator();
rootToLeafPrinter(rootPtr);
return 0;
}



