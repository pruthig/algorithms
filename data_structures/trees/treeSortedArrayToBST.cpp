// This program converts a sorted array to a binary search tree
#include<iostream>
#include<vector>


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
void traverseNodes(treeStruct *ptr) {
    if(ptr == nullptr)
        return;
	traverseNodes(ptr->left);
	cout<<ptr->element<<" ";
	traverseNodes(ptr->right);
}

treeStruct* createTreeFromSorted(vector<int>& vec, int begin, int end)
{
    if(vec.size() == 0 || begin > end)
        return nullptr;

    int mid = begin+ (end-begin)/2;
    treeStruct* root = newNode(vec.at(mid));
    root->left = createTreeFromSorted(vec, begin, mid-1);
    root->right = createTreeFromSorted(vec, mid+1, end);
    return root;
}

int main()
{
    vector<int> vec{ 2, 3, 6, 8, 10, 12, 34, 46, 49, 51, 69, 100};
	treeStruct *root = createTreeFromSorted(vec, 0, vec.size()-1);
    traverseNodes(root);
    return 0;
}



