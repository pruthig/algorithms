#include<iostream>

using namespace std;

struct TreeNode{
	int val;
	struct TreeNode *left;
	struct TreeNode *right;
};

struct TreeNode* newNode(int data)
{
	struct TreeNode *newElement = new(struct TreeNode);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->val = data;
	return newElement;
}

struct TreeNode* treeCreator(){
	//create the main pointer
	struct TreeNode *mainPtr;
	mainPtr = newNode(10);
	mainPtr->left = newNode(6);
	mainPtr->right = newNode(6);
	
	mainPtr->left->left = newNode(3);
	mainPtr->left->right = newNode(4);
	
	mainPtr->right->left = newNode(4);
	mainPtr->right->right = newNode(3);
	
	//mainPtr->right->right->left = newNode(15);
	//mainPtr->right->right->right = newNode(20);
	
	return mainPtr;
}

bool IsTreeSymmetric(TreeNode *ptr_l, TreeNode *ptr_r)
{
	if(!ptr_l&&!ptr_r)
		return true;
	if( (ptr_l && !ptr_r) || (!ptr_l && ptr_r))
		return false;
	return ( (ptr_l->val == ptr_r->val) && 
	         (IsTreeSymmetric(ptr_l->left, ptr_r->right)) && IsTreeSymmetric(ptr_l->right, ptr_r->left) );
}
	
	

int main()
{
	struct TreeNode *rootPtr = treeCreator();
	bool val = IsTreeSymmetric(rootPtr->left, rootPtr->right);
	if(val)
		cout<<"Tree is symmetric\n";
	else
		cout<<"Tree is not symmetric\n";
	return 0;
}



