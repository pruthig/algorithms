     /*1 -> NULL
       /  \
      2 -> 3 -> NULL
     / \  / \
    4->5->6->7 -> NULL





























*/    

#include<iostream>
#include<queue>

using namespace std;

struct TreeLinkNode {
  int val;
  TreeLinkNode *left, *right, *next;
  TreeLinkNode(int x) : val(x), left(NULL), right(NULL), next(NULL) {}
};


namespace {
	queue<TreeLinkNode* > q;
};

void printTree(TreeLinkNode* root) {
	if(!root)
		return;
	
	printTree(root->left);
	cout<<" Node: "<<root->val;
	if(root->next)
		cout<<" Node->next: "<<root->next->val;
	cout<<endl;
	printTree(root->right);
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

TreeLinkNode* treeCreator()
{
	//create the main pointer
	TreeLinkNode *mainPtr;
	mainPtr = new TreeLinkNode(10);
	mainPtr->left = new TreeLinkNode(6);
	mainPtr->right = new TreeLinkNode(12);

	mainPtr->left->left = new TreeLinkNode(2);
	mainPtr->left->right = new TreeLinkNode(9);

	mainPtr->right->left = new TreeLinkNode(11);
	mainPtr->right->right = new TreeLinkNode(18);

	mainPtr->right->right->left = new TreeLinkNode(16);
	mainPtr->right->right->right = new TreeLinkNode(20);
	
	return mainPtr;
}

 
// function to parse tree and setting up  the pointers
void populateNextPointers(TreeLinkNode *node) {
	TreeLinkNode *prevNode = nullptr;
	// Add first element
	if(!node)
		return;
	q.push(node);
	// try once
	int count = 1;
	int temp = 0;
	while(!q.empty()) {

		prevNode = nullptr;
		for(int i = 0; i < count; ++i) {
			TreeLinkNode *tmpNode = q.front();
			q.pop();
			if(tmpNode->left) {
				if(prevNode) {
					prevNode->next = tmpNode->left;
				}
				prevNode = tmpNode->left;
				q.push(tmpNode->left);
				++temp;
			}
			if(tmpNode->right) {
				if(prevNode) {
					prevNode->next = tmpNode->right;
				}
				prevNode = tmpNode->right;
				q.push(tmpNode->right);
				++temp;
			}
			
		}
		count = temp;
		temp = 0;
			
	}
}

int main() {
	TreeLinkNode *root = treeCreator();
	populateNextPointers(root);
	printTree(root);
	
	return 0;
}










