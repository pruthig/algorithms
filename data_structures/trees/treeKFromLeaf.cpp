// This tree prints all the nodes which are at 'k' distance from leaf
#include<iostream>


using namespace std;

int g_count = 0;

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
int printKFromLeaf(struct treeStruct *node, int k)
{
    if(node==NULL)
    return -1;
    
    int x=1+printKFromLeaf(node->left,k);
    int y=1+printKFromLeaf(node->right,k);
    
    cout<<"x and y are : "<<x<<" "<<y<<endl;
    if(x==k || y==k)
    cout<<node->element<<"   ";

}

int main()
{   
    int k = 0;
	struct treeStruct *rootPtr = treeCreator();
    cout<<"Enter k\n";
    cin>>k;
    cout<<"The nodes are :\n";
    
	printKFromLeaf(rootPtr, k);
	return 0;
}



