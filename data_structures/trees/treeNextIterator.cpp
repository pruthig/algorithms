// This program creates a tree and implement 2 methods .. next and hasNext().
// First next() call will return smallest element in the tree and next next()
// call will return 2nd smallest element in tree

#include<iostream>
#include<stack>

using namespace std;

typedef struct treeStruct
{
	int element;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;


namespace {
    stack<treeStruct*> st;
}


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

// return false if pointer is NULL
bool initialize(treeStruct *root) {
    if(!root)
        return false;

    while(root) {
        st.push(root);
        root = root->left;
    }
};

bool hasNext() {
    return st.size();
}

int next() {
    if(st.empty())
        return -1;
    treeStruct *node = st.top();
    treeStruct *right = node->right;

    st.pop();
    while(right) {
        st.push(right);
        right = right->left;
    };
    return node->element;
}


int main()
{
	struct treeStruct *rootPtr = treeCreator();
    initialize(rootPtr);
    cout<<next()<<endl;

    cout<<next()<<endl;
    cout<<next()<<endl;
    cout<<next()<<endl;
	return 0;
}



