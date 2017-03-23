//The program is to to sum the numbers created from root to leaf traversal of nodes..

#include<iostream>
#include<cmath>


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
mainPtr = newNode(8);
mainPtr->left = newNode(6);
mainPtr->right = newNode(2);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(1);
mainPtr->right->right = newNode(8);

mainPtr->right->right->left = newNode(5);
mainPtr->right->right->right = newNode(0);

return mainPtr;
}

int sum = 0;
int counter = 0;
char arr[20] = {-1};

void countNodes(struct treeStruct *ptr){
if(ptr->left == NULL || ptr->right == NULL)
{
	arr[counter++] = ptr->element; 
	//count the number of nodes in array...
	int tmp = 0;
	for(int i=1;i<=counter;i++)
		tmp = tmp + arr[i-1] * pow(10, counter-i);
	sum = sum + tmp;
	return;
}
	
arr[counter++] = ptr->element; 
countNodes(ptr->left);
--counter;
countNodes(ptr->right);
--counter;
}

int main(){
struct treeStruct *rootPtr = treeCreator();
//int count = 
countNodes(rootPtr);
cout<<"Number of nodes in tree : "<<sum<<endl;
return 0;
}



