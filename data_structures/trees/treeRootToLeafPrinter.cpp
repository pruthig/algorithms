//Print paths from root to leaf...
#include<iostream>


using namespace std;

int arr[100] = {0};

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
void printArray(int index) {
    for(int i = 0 ; i <= index; ++i)
        cout<<arr[i]<<" ";
    cout<<endl;
}

void traverseNodes(struct treeStruct *ptr, int ind)
{
    if(ptr != NULL)
        arr[ind] = ptr->element;
    if(ptr->left == NULL && ptr->right == NULL) {
        printArray(ind);
        return;
    }

    traverseNodes(ptr->left, ind+1);
    traverseNodes(ptr->right, ind+1);
}

int main()
{
    struct treeStruct *rootPtr = treeCreator();
    traverseNodes(rootPtr, 0);
    return 0;
}



