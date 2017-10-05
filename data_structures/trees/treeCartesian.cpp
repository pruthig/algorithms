//This program creates a copy of tree using recursion..
//logic >> create parent node then respective left and right calls for creation 
//Initially pass the address of the root node of the new tree to reflect the changes in the tree

#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

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


void rootToLeafPrinter(treeStruct *ptr){
	if(ptr == NULL){
		return;
	}
	rootToLeafPrinter(ptr->left);  //6 ka left completed
	cout<<ptr->element<<" ";
	rootToLeafPrinter(ptr->right);
}


// r refers to index after the end
struct treeStruct* treeCreator(vector<int>& arr, int l, int r){
	//create the main pointer
	if(l >= r)
		return nullptr;
	if(l == r) {
		treeStruct *new_node = newNode(arr[l]);
		return new_node;
	}
	// distance(A, max_element(A, A + N))  <-- Finds the max element
	int max_index = distance(arr.begin(), max_element(arr.begin()+l, arr.begin()+r));
	treeStruct *new_node = newNode(arr[max_index]);
	new_node->left = treeCreator(arr, l, max_index);
	new_node->right = treeCreator(arr, max_index+1, r);
	return new_node;
}


int main(){
	vector<int> vec{ 2, 1, 3};
	treeStruct *rootPtr = treeCreator(vec, 0, vec.size());
	rootToLeafPrinter(rootPtr);
	return 0;
}



