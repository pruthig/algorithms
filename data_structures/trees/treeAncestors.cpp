//Find all ancestors of a node.. this program will find all ancestors of a node
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
    for(int i = 0 ; i < index; ++i)
        cout<<arr[i]<<" ";
    cout<<endl;
}

void findAncestors(struct treeStruct *ptr, int ind, int key)
{
    if(ptr == NULL)
        return;
    else {
        if(ptr->element == key) {
            printArray(ind);
            return; 
        }
        
        arr[ind] = ptr->element;
    }

    findAncestors(ptr->left, ind+1, key);
    findAncestors(ptr->right, ind+1, key);
}

int main()
{
    struct treeStruct *rootPtr = treeCreator();
    int key;
    cout<<"Enter key"<<endl;
    cin>>key;
    findAncestors(rootPtr, 0, key);
    return 0;
}



