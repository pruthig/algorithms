// This program prints all root->leaf paths using deque
#include<iostream>
#include<deque>
#include<iterator>

using namespace std;

typedef struct treeStruct{
	int element;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;

namespace {
	deque<int> q_int;
}

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
	
	return mainPtr;
}

void rootToLeafPrinter(struct treeStruct *ptr)
{
	if(ptr == NULL)
		return;
	q_int.push_back(ptr->element);
	if(ptr->left == NULL && ptr->right == NULL) {
		copy(begin(q_int), end(q_int), std::ostream_iterator<int>(cout, " "));
		cout<<endl;
		q_int.pop_back();
		return;
	}
	
	rootToLeafPrinter(ptr->left);
	rootToLeafPrinter(ptr->right);
	
	q_int.pop_back();
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	rootToLeafPrinter(rootPtr);
	return 0;
}



