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

void nodeLevelCousin(struct treeStruct *ptr, int node1, int node2)
{
    int local = 0;
    bool node1_found = false, node2_found = false;
    int parent1 = 0, parent2 = 0;
    struct treeStruct *dummy  = NULL;
    q.push(ptr);

    while(!q.empty())
    {
        local = q.size();
        node1_found = false;
        node2_found = false;
        parent1 = 0;
        parent2 = 0;

        for(int i=0; i < local ; i++)
        {
            dummy = q.front();
            q.pop();

            if(dummy->left != NULL) {
                if(dummy->left->element == node1) {
                    node1_found = true;
                    parent1 = dummy->element;
                }
                 if(dummy->left->element == node2) {
                    node2_found = true;
                    parent2 = dummy->element;
                }

                q.push(dummy->left);
            }
            if(dummy->right != NULL) {
                if(dummy->right->element == node1) {
                    node1_found = true;
                    parent1 = dummy->element;
                }
                 if(dummy->right->element == node2) {
                    node2_found = true;
                    parent2 = dummy->element;
                }
                q.push(dummy->right);
            }
        }
        if(node1_found && node2_found && parent1 != parent2) 
        {
            cout<<"Nodes are cousins\n";    
            return;
        }

    }
    cout<<"Either nodes not found or not cousins\n";
}

int main(){
    int node1, node2;
    struct treeStruct *rootPtr = treeCreator();
    cout<<"Enter the two nodes, for whom you want to make cousin check\n";
    cin>>node1>>node2;
    nodeLevelCousin(rootPtr, node1, node2);
    return 0;
}



