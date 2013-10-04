//tree reverse mirror change that tree so that all nodes are swapped.. in original

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

rootToLeafPrinter(ptr->left);  //6 ka left completed
pop(ptr->left);
rootToLeafPrinter(ptr->right);
pop(ptr->right);

return;
}

void swapper(struct treeStruct *ptr){
struct treeStruct *dummy;
dummy = ptr->left;
ptr->left = ptr->right;
ptr->right = dummy;
return;
}

void reversal(struct treeStruct *ptr){
if(ptr == NULL && ptr == NULL)
return;

reversal(ptr->left);
reversal(ptr->right);
swapper(ptr);
}











int main(){
std::fill_n(arr, 10000, -1);
struct treeStruct *rootPtr = treeCreator();
rootToLeafPrinter(rootPtr);
reversal(rootPtr);
cout<<endl<<endl<<"After reversal tree :"<<endl<<endl;
rootToLeafPrinter(rootPtr);
return 0;
}



