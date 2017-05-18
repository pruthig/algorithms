// this program finds out the nodes under which number of children are 'k'
// where 'k' is taken as input

#include<iostream>
#include<climits>


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

int findKLeavesNodes(struct treeStruct *ptr, int k)
{
    if(ptr == NULL)
        return 0;
	if(ptr->left == NULL && ptr->right == NULL)
		return 1;

	int sum = findKLeavesNodes(ptr->left, k) + findKLeavesNodes(ptr->right, k);

    if(sum == k)
        cout<<"\nNode is : "<<ptr->element<<endl;
    return sum+1;
}

int main()
{
    int k = 0;
	struct treeStruct *rootPtr = treeCreator();
    cout<<"Enter the number k which will be the number of children\n";
    cin>>k;
	findKLeavesNodes(rootPtr, k);
	return 0;
}



