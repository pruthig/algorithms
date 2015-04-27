//The aim is gto find the next in-order successor of the tree ( BST ) ... thats it
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
int counter = 0;
bool found = false;
int node;
void searchNodes(struct treeStruct *ptr, int p)
{
	if(ptr== NULL)
		return;

	searchNodes(ptr->left, p);
	if(found == true){
		node = ptr->element;
		found = false;
	}

	if(ptr->element == p)
		found = true;
	
	searchNodes(ptr->right, p);

}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int t;
	cout<<"Enter the node of which i-o successor to be searched :";
	cin>>t;
	searchNodes(rootPtr, t);
	cout<<"Solution is  : "<<node<<endl;
	return 0;
}



