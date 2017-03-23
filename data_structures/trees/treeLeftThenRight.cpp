//Base tree ... totally bare
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

void traverseNodes(struct treeStruct *head,struct treeStruct *ptr, int *i)
{
	if(ptr == NULL ) {
		if(*i == 0)
			*i = 2;
		return;
	}

    if(*i == 0) {
        cout<<"\nInside *i == 0\n";
		cout<<ptr->element<<", ";
		ptr = ptr->left;
		traverseNodes(head, ptr, i);
	}
	if(*i == 2){

    	cout<<"\nInside *i == 2\n";
		*i = 1;
		ptr = head->right;
	}

    cout<<"\nInside *i == 1\n";
	cout<<ptr->element<<", ";
   	ptr = ptr->right;
	traverseNodes(head, ptr, i);

}

int main()
{
    //flag 0 means left else it is right
	struct treeStruct *rootPtr = treeCreator();
	int i = 0;
	traverseNodes(rootPtr, rootPtr->left, &i);
	return 0;
}



