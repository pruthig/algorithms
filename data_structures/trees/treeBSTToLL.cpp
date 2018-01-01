// This program will convert a tree to Linked list (in-place) the order in tree will be taken as pre-order traversal...
#include<iostream>
#include<stack>

using namespace std;

typedef struct treeStruct
{
	int val;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;


treeStruct* newNode(int data)
{
	struct treeStruct *newElement = new(struct treeStruct);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->val = data;
	return newElement;
}

treeStruct* treeCreator()
{
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

/*
                                        10
                                      /    \
                                    6      12
                                   / \     / \
                                  2   9   11  18
                                              / \
                                            16   20
*/

void printLL(treeStruct *rootPtr) {
	while(rootPtr) {
		cout<<rootPtr->val<<" ";
		rootPtr = rootPtr->right;
	}
}

void preorderBTToLL(treeStruct *ptr)
{
	if(ptr == NULL)
		return;
	
	stack<treeStruct *> st;
	treeStruct *temp = NULL;
	// After this loop all right-inclined nodes will be in the stack...
	
	while(ptr || !st.empty()) {
		while(ptr) {
			if(ptr->right) {
				st.push(ptr->right);
			}
			temp = ptr;
			ptr = ptr->left;
			temp->left = NULL;
			temp->right = ptr;
		}
		if(!st.empty()) {
			if(temp)
				temp->right = st.top();
			ptr = st.top();
			st.pop();
		}
	}
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	preorderBTToLL(rootPtr);
	printLL(rootPtr);
	return 0;
}



