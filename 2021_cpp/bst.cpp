#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>
#include <cmath>
#include <set>
#include <map>

using namespace std; 

struct Node
{
    int data;
    struct Node* left;
    struct Node* right;
    
    Node(int x){
        data = x;
        left = right = NULL;
    }
};

// Return the root of the modified BST after deleting the node with value X
Node *deleteNode(Node *root,  int X)
{
    if(!root)
        return nullptr;
    if(root->data < X)
        root->right = deleteNode(root->right, X);
    else if(root->data > X)
        root->left = deleteNode(root->left, X);
    else {
        if(!root->left && !root->right) {
            delete root;
            return nullptr;
        }
        else if(root->left && root->right) {
            // find inorder successor
            Node *t = root->right;
            while(t->left != nullptr)
                t = t->left;
            root->data = t->data;
            root->right = deleteNode(root->right, t->data);
            return root;
        }
        else if(root->left || root->right) {
            if(root->left) {
                Node *tmp = root->left;
                delete root;
                root = nullptr;
                return tmp;
            }
            else {
                Node *tmp = root->right;
                delete root;
                root = nullptr;
                return tmp;
            }
        }
    }
}

// Check if a tree is BST or not
bool isBSTUtil(Node *root, int min_,  int max_) {
    if(!root)
        return true;
    int rd = root->data;
    if(rd < min_ || rd > max_)
        return false;
    return isBSTUtil(root->left, min_, root->data-1) &&
    isBSTUtil(root->right, root->data+1, max_);
}

bool isBST(Node* root) 
{
    return isBSTUtil(root, INT_MIN, INT_MAX);
}

// Construct tree from postorder traversal
Node *constructTreeUtil(int pre[], int *i, int size, int mn, int mx) {
    if(*i == size)
        return nullptr;
    if(pre[*i] > mn && pre[*i] < mx) {
        int datum = pre[*i];
        Node *n = new Node(datum);
        ++(*i);
        n->left = constructTreeUtil(pre, i, size, mn, datum);
        n->right = constructTreeUtil(pre, i, size, datum, mx);
        return n;
    }
    else
        return nullptr;
}

Node* constructTree(int pre[], int size) {
    int i = 0;
    return constructTreeUtil(pre, &i, size, INT_MIN, INT_MAX);
}


int main() {
    return 0;
}