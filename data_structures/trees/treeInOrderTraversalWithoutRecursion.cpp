#include<iostream>
#include<stack>
#include<vector>

using namespace std;

struct treeStruct{
	int element;
	struct treeStruct *left;
	struct treeStruct *right;
};

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
	
	mainPtr->right->right->left = newNode(15);
	mainPtr->right->right->right = newNode(20);
	
	return mainPtr;
}

void InorderTraversalWithoutRecur(struct treeStruct *ptr) {
	if(ptr == nullptr)
		return;
	stack<treeStruct*> st{};
	treeStruct *temp = ptr;
	while(temp) {
		st.push(temp);
		temp = temp->left;
	}
	while(!st.empty()) {
		treeStruct *p = st.top();
    cout<<p->element<<" ";
		treeStruct *p_rt = p->right;
		st.pop();
		while(p_rt) {
			st.push(p_rt);
			p_rt = p_rt->left;
		}
	}
}
	
	

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	InorderTraversalWithoutRecur(rootPtr);
	return 0;
}



