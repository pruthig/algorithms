//We can print "ancestor" using a preorder traversal and an array which can be used to store the in depth pattern



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
mainPtr = newNode(1);
mainPtr->left = newNode(2);
mainPtr->right = newNode(3);

mainPtr->left->left = newNode(4);
mainPtr->left->right = newNode(5);

mainPtr->right->left = newNode(6);
mainPtr->right->right = newNode(7);

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

void rootToLeafPrinter(struct treeStruct *ptr, int nodeToFind){
if(ptr == NULL){
	return;
}
else
{	Counter++;
	arr[Counter] = ptr->element;
}

if(ptr->left == NULL && ptr->right == NULL && ptr->element == nodeToFind){
	print();
	return;
}

rootToLeafPrinter(ptr->left, nodeToFind);  //6 ka left completed
pop(ptr->left);
rootToLeafPrinter(ptr->right, nodeToFind);
pop(ptr->right);

return;
}

int main(){
int nodeToFind;
std::fill_n(arr, 10000, -1);
struct treeStruct *rootPtr = treeCreator();
cout<<"Enter the node to search : "<<endl;
cin>>nodeToFind;
rootToLeafPrinter(rootPtr, nodeToFind);
return 0;
}



