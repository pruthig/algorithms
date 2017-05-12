// This program finds out if 2 nodes are cousins. Cousins are the nodes which are at same level and not parent of each other.
#include<iostream>
#include<queue>


using namespace std;

typedef struct treeStruct{
    int element;
    struct treeStruct *left;
    struct treeStruct *right;
}treeStruct;

queue<treeStruct*> q;

struct treeStruct* newNode(int data)
{
    struct treeStruct *newElement = new(struct treeStruct);
    newElement->left = NULL;
    newElement->right = NULL;
    newElement->element = data;
    return newElement;
}

struct treeStruct* treeCreator(){
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

    mainPtr->left->left->right = newNode(6);
    return mainPtr;
}

/*
                                        10
                                      /    \
                                    6      12
                                   / \     / \
                                  2   9   11  18
                                   \          / \
                                    6       16   20
*/

void treeDiagonalSum(struct treeStruct *ptr)
{
    int local = 0;
    int level = 0, sum = 0;
    struct treeStruct *dummy  = NULL, *ponder = NULL;
    // push first level
    while(ptr) {
        q.push(ptr);
        ptr = ptr->right;
    }
    
    

    while(!q.empty())
    {
        local = q.size();
        sum = 0;
        ++level;

        for(int i=0; i < local ; i++)
        {
            dummy = q.front();
            ponder = dummy;
            q.pop();
            sum += dummy->element;
            

            if(dummy->left != NULL) {
                q.push(dummy->left);
                ponder = dummy->left->right;
                while(ponder) {
                    q.push(ponder);
                    ponder = ponder->right;
                }
                
            }
        }
        cout<<"Sum at level "<<level<<"# "<<sum<<endl;
    }
}

int main(){
    struct treeStruct *rootPtr = treeCreator();
    treeDiagonalSum(rootPtr);
    return 0;
}



