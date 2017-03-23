//This is another method to find inorder traversal, the crux of the method is that if the right child of
//node is NULL then next successor will be the immediate parent whose left child is thi node...In the below example, 9 has 10
//as its successor
#include<iostream>


using namespace std;

typedef struct treeStruct
{
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

void find_t(treeStruct *root, treeStruct **temp, int t){
	if(root == NULL)
		return;

	find_t(root->left, temp, t);
	if(root->element == t)
		*temp = root;
	find_t(root->right, temp, t);
}
void searchNodes(struct treeStruct *r, int p)
{
	struct treeStruct *p_pointer = NULL;
	struct treeStruct *ptr = r;

	find_t(ptr, &p_pointer, p);
	if(p_pointer == NULL){
		cout<<"Element not found\n";
		return;
	}
	if(p_pointer->right != NULL){
		p_pointer = p_pointer->right;
		while(p_pointer->left != NULL)
			p_pointer = p_pointer->left;
		cout<<"Successor is : "<<p_pointer->element;
		return;
	}
	
	//2nd case
	struct treeStruct *successor = NULL;
	while(ptr)
	{
		if( p < ptr->element ){
			successor = ptr;
			ptr = ptr->left;
		}
		else if(p > ptr->element)
			ptr = ptr->right;
		else
			break;
	}
	cout<<"Next Successor is :"<<successor->element;

}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int t;
	cout<<"Enter the node of which i-o successor to be searched :";
	cin>>t;
	searchNodes(rootPtr, t);
	return 0;
}



