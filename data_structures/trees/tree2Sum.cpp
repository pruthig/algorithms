// This program creates a 2-Sum Binary Tree
#include<iostream>
#include<set>

using namespace std;


namespace {
	set<int> st;
	//using vt =;
};

typedef struct treeStruct
{
	int val;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
	struct treeStruct *newElement = new(struct treeStruct);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->val = data;
	return newElement;
}

struct treeStruct* treeCreator()
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


int traverseNodes(struct treeStruct *ptr, int k)
{
	if(ptr == NULL)
		return 0;
	
	set<int>::iterator iter = st.find(k-(ptr->val));
	if(iter != st.end())
		return 1;
	else
		st.insert(ptr->val);
		
	return traverseNodes(ptr->left, k) || traverseNodes(ptr->right, k);
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int k = 0;
	cout<<"Enter 2-Sum value: \n";
	cin>>k;
	if(traverseNodes(rootPtr, k))
		cout<<"Found the sum\n";
	else
		cout<<"Sum not found\n";
	return 0;
}



