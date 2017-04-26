//Find if there exists sum path from root to leaf equal to given input
#include<iostream>


using namespace std;

typedef struct treeStruct
{
    int element;
    struct treeStruct *left;
    struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int element)
{
    struct treeStruct *newElement = new(struct treeStruct);
    newElement->left = NULL;
    newElement->right = NULL;
    newElement->element = element;
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

bool hasSomePath(struct treeStruct *node, int input)
{
    if(node == NULL)   
        return false;
    if(node->element == input || hasSomePath(node->left, input-(node->element)) || 
        hasSomePath(node->right, input-(node->element)))
        return true;
    else
        return false;
    
}
int main()
{
    int input;
    struct treeStruct *rootPtr = treeCreator();
    cout<<"Enter the sum"<<endl;
    cin>>input;
    cout<<"Is there some path? -> "<<boolalpha<<hasSomePath(rootPtr, input)<<endl;
    return 0;
}



