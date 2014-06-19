#include<iostream>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;

void print();

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

mainPtr->right->right->left = newNode(16);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int arr[99] = {-1};
int count = -1;

void traverse(struct treeStruct *ptr, int node){

if(ptr == NULL)
	return;
if(ptr->element == node){
	print();
	return;
}

if(ptr != NULL)
	arr[++count] = ptr->element;
	
traverse(ptr->left,  node);
traverse(ptr->right, node);
--count;

}

void print(){
for(int i=0;i<=count;i++)
	cout<<arr[i]<<", ";
cout<<endl;
}

int main(){
int node = 0;
cout<<"enter the input\n";
cin>>node;
struct treeStruct *rootPtr = treeCreator();
traverse(rootPtr, node);
return 0;
}



