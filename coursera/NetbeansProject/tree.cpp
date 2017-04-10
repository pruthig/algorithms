/*
 * Copyright 2017 Gaurav Pruthi.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include<iostream>
#include <queue>

using namespace std;


typedef struct TreeNode {
    int value;
    TreeNode *left;
    TreeNode *right;

    TreeNode(){}
    TreeNode(int val){
        value = val;
        left = NULL;
        right = NULL;
    }
}TreeNode;

class Tree {
    TreeNode *root;
public:
    Tree(int val);
    ~Tree();
    void setRoot(TreeNode *root);
    TreeNode* getRoot();
    void populateTree();
    void traverseInorder(TreeNode *root);
    int findTreeSize(TreeNode *node);
    void maxDepth(TreeNode *node, int count, int *max);
    void traverseLevelOrder(TreeNode *root);
    void printRootToLeaf();
};

TreeNode* Tree::getRoot(){
    return this->root;
}

void Tree::setRoot(TreeNode* root) {
    this->root = root;
}

Tree::Tree(int val = 0) {
    root = new TreeNode;
    root->value = val;
    root->left = NULL;
    root->right = NULL;
}

Tree::~Tree(){
    delete root;
    root = nullptr;
}

void Tree::traverseInorder(TreeNode *node) {
    if(node == NULL)
        return;
    traverseInorder(node->left);
    cout<<" Value of node is: "<<node->value<<", ";
    traverseInorder(node->right);
}

int Tree::findTreeSize(TreeNode *node){
    if(node == NULL)
        return 0;
    return findTreeSize(node->left) + 1 + findTreeSize(node->right);
}
void Tree::populateTree() {
    root->value = 25;
    root->left = new TreeNode(20);
    root->right = new TreeNode(30);
    
    root->left->left = new TreeNode(18);
    root->left->right = new TreeNode(22);
    
    root->left->left->left = new TreeNode(17);
    
    root->right->left = new TreeNode(28);
    root->right->right = new TreeNode(32);
}

void Tree::maxDepth(TreeNode *node, int count, int *max) {
    if(node == NULL) {
        if(count > *max){
            *max = count;
        }
        return;
    }
    maxDepth(node->left, count+1, max);
    maxDepth(node->right, count+1, max);
}

void Tree::traverseLevelOrder(TreeNode *root) {
    // Level order traversal using Queue
    std::queue<TreeNode*> q;
    TreeNode *temp = NULL;
    q.push(root);
    
    while(!q.empty()){
        temp = q.front();
        q.pop();
        cout<<temp->value<<", ";
        if(temp->left)
            q.push(temp->left);
        if(temp->right)
            q.push(temp->right);
        
    }
}

void printRootToLeafUtil(TreeNode *node, int arr[], int counter) {
    if(node->left == NULL && node->right == NULL) {
        int i = 0;
        arr[counter] = node->value;
        while(arr[i] != 0 && i <= counter) {
            cout<<arr[i]<<", ";
            i++;
        }
        cout<<endl;
        arr[counter] = 0;
        return;
    }
    arr[counter++] = node->value;
    
    if(node->left)
        printRootToLeafUtil(node->left, arr, counter);
    if(node->right)
        printRootToLeafUtil(node->right, arr, counter);
    
    arr[counter--] = 0;
}

// To print root to leaf paths, use an array where you'll add the elements from the end
void Tree::printRootToLeaf() {
    int arr[50] = { 0 };
    //int counter = 0;
    
    ::printRootToLeafUtil(getRoot(), arr, 0);
}

void tree_main() {
    int match;
    Tree *tree = new Tree();
    int size = 0;
    int maxDepth = 0;
    tree->populateTree();
    while(1)
    {
        cout<<"\n\nWhat do u want to test"<<'\n';
        cout<<"101. Tree traversal - Inorder"<<'\n';
        cout<<"102. Tree traversal - Level order"<<'\n';
        cout<<"103. Find tree Size"<<'\n';
        cout<<"104. Find Max Depth"<<'\n';
        cout<<"105. Print Root->Leaf"<<'\n';
        cout<<"0. Exit"<<'\n';
        cout<<"\nEnter your choice\n";
        cin>>match;

        switch(match) {
            case 101:
                tree->traverseInorder(tree->getRoot());
                break;
            case 102:
                tree->traverseLevelOrder(tree->getRoot());
                break;
            case 103:
                size = tree->findTreeSize(tree->getRoot());
                cout<<"Tree Size is:"<<size<<"\n";
                break;
            case 104:
                tree->maxDepth(tree->getRoot(), 0, &maxDepth);
                cout<<"Max Depth is:"<<maxDepth<<"\n";
                break;
            case 105:
                tree->printRootToLeaf();
                break;
            case 0:
            default:
                exit(0);
        }
    }
}

