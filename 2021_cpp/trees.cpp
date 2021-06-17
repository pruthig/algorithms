#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>
#include <utility>
#include <cmath>
#include <set>
#include <map>

using namespace std; 


/*** LCA in Binary tree ******/

/*********** Follow this optimized solution *********
if(root->data == n1 || root->data == n2)
return root;
Node* lca1 = lca(root->left, n1, n2);
Node* lca2 = lca(root->right, n1, n2);

if(lca1 != NULL && lca2 != NULL) return root;
else if(lca1 != NULL)  return lca1;
else return lca2;
/**********************************/

struct Node
{
    int data;
    struct Node *left;
    struct Node *right;
    struct Node *nextRight;
    
    Node(int x){
        data = x;
        left = right = nextRight = NULL;
    }
};
 
Node* fillMapWithN1(Node *root, set<Node*>& st, int n1 ) {
    if(!root)
        return nullptr;
    if(root->data == n1) {
        st.insert(root);
        return root;
    }
    Node *lptr = fillMapWithN1(root->left, st, n1);
    if(lptr != nullptr) {
        st.insert(root);
        return root;
    }
    Node *rptr = fillMapWithN1(root->right, st, n1);
    if(rptr != nullptr) {
        st.insert(root);
        return root;
    }
    return nullptr;
}

Node* searchForN2(Node* root, set<Node*>& st, int n2, Node*& res) {
    if(!root)
        return nullptr;
    if(root->data == n2) {
        if(!res) {
            auto iter = st.find(root);
            if(iter != st.end())
                res = *iter;
        }
        return root;
    }
    Node *lptr = searchForN2(root->left, st, n2, res);
    if(lptr) {
        if(!res) {
            auto iter = st.find(root);
            if(iter != st.end())
                res = *iter;
        }
        return root;
    }
    Node *rptr = searchForN2(root->right, st, n2, res);
    
    if(rptr) {
        if(!res) {
            auto iter = st.find(root);
            if(iter != st.end())
                res = *iter;
        }
        return root;
    }
    return nullptr;
}
// End of custom implementation. Use the optimized implementation as mentioned above

// Top class implementation of finding the diameter .. 0.69 -> 0.37 seconds
std::pair<int, int> diameterUtil(Node *root, int& mx) {
     if(!root)
        return std::make_pair(0, 0);
    std::pair<int, int> ld_h = diameterUtil(root->left, mx);
    std::pair<int, int> rd_h = diameterUtil(root->right, mx);
    
    int lh = ld_h.second;
    int rh = rd_h.second;
    
    mx = max(1+lh+rh, max(ld_h.first, rd_h.first));
    return std::make_pair(1+lh+rh, 1+max(lh, rh));
}

int diameter(Node* root) {
    int mx_diam = 0;
    diameterUtil(root, mx_diam );
    return mx_diam;
}

/* If n1 and n2 are present, return pointer
   to LCA. If both are not present, return 
   NULL. Else if left subtree contains any 
   of them return pointer to left.*/

Node* lca(Node* root ,int n1 ,int n2 )
{
   set<Node*> st{};
   Node* res = nullptr;
   fillMapWithN1(root, st, n1);
   if(st.empty())
    return nullptr;
   searchForN2(root, st, n2, res);
   return res;
   
}

// Non-recursive solution of pre-order traversal
vector <int> preorder(Node* root) {
  vector<int> vec{};
  stack<Node*> st{};
  st.push(root);
  while(!st.empty()) {
      Node *t = st.top();
      st.pop();
      while(t) {
        if(t->right)
            st.push(t->right);
        vec.push_back(t->data);
        t = t->left;
      }
  }
  return vec;
}

// Non-recursive solution of post-order traversal
vector <int> postorder(Node* root) {
  vector<int> vec{};
  stack<Node*> st{};
  st.push(root);
  while(!st.empty()) {
      Node *t = st.top();
      st.pop();
      while(t) {
        if(t->left)
            st.push(t->left);
        vec.push_back(t->data);
        t = t->right;
      }
  }
  return vec;
}

/**** Check if a tree is balanced or not ***********/

int isBalancedUtil(Node *root, bool& isBalancedFlag) {
    if(!root)
        return 0;
    if(!root->left && !root->right)
        return 1;
    int l_sum = isBalancedUtil(root->left, isBalancedFlag);
    int r_sum = isBalancedUtil(root->right, isBalancedFlag);
    
    if(abs(l_sum-r_sum)>1)
        isBalancedFlag = false;
    
    return 1 + max(l_sum, r_sum);
}

// This function should return tree if passed  tree 
// is balanced, else false. 
bool isBalanced(Node *root)
{
    bool isBalancedFlag = true;
    isBalancedUtil(root, isBalancedFlag);
    return isBalancedFlag;
}

// Inorder traversal in iterative manner
void inOrder(Node* root)
{
    stack<Node*> s;
    auto curr = root;
    while(!s.empty() || curr) {
      if(curr) {
          s.push(curr);
          // Going left.
          curr = curr->left;
      } else {
          // Going up
          curr = s.top();
          s.pop();
          cout << curr->data << " ";
          // Going right
          curr = curr->right;
      }
    }
}

// Check whether all of its Nodes have the value 
// equal to the sum of their child Nodes.
int isSumProperty(Node *root)
{
    if(!root)
        return true;
    if(!root->left && !root->right)
        return true;
    int l_sum = root->left?root->left->data:0;
    int r_sum = root->right?root->right->data:0;
    if(root->data != l_sum + r_sum)
        return false;
    else
        return isSumProperty(root->left) && isSumProperty(root->right);
}

// program to check if tree is foldable or not
bool validation(deque<Node*>& dq) {
    if(dq.size()%2 == 1)
        return false;
    while(!dq.empty()) {
        Node *l = dq.front();
        Node *r = dq.back();
        dq.pop_back();
        dq.pop_front();
        if((!l->left && r->right) || (!l->right && r->left) ||
           (l->left && !r->right) || (l->right && !r->left))
           return false;
    }
    return true;
}

bool IsFoldable(Node* root)
{
    // Your code goes here
    queue<Node*> q{};
    deque<Node*> dq{};
    if(!root)
        return true;
    else if(root->left && !root->right ||
            root->right && !root->left){
                return false;
    }
    else {
        if(root->left)
            q.push(root->left);
        if(root->right)
            q.push(root->right);
    }
    while(!q.empty()) {
      int sz = q.size();
      while(sz--) {
          Node *front = q.front();
          q.pop();
          dq.push_back(front);
          if(front->left)
            q.push(front->left);
          if(front->right)
            q.push(front->right);
          
      }
      if(validation(dq) == false)
        return false;
      
    }
    return true;
}

// Optimized method to check if a tree is foldable or not
bool IsFoldableUtil(Node* n1, Node* n2)
{
    /* If both left and right subtrees are NULL,
    then return true */
    if (n1 == NULL && n2 == NULL) {
        return true;
    }
 
    /* If one of the trees is NULL and other is not,
    then return false */
    if (n1 == NULL || n2 == NULL) {
        return false;
    }
 
    /* Otherwise check if left and right subtrees are
    mirrors of their counterparts */
    return IsFoldableUtil(n1->left, n2->right)
           && IsFoldableUtil(n1->right, n2->left);
}

/* Returns true if the given tree can be folded */
bool IsFoldableOptimized(Node* root)
{
    if (root == NULL) {
        return true;
    }
 
    return IsFoldableUtil(root->left, root->right);
}

// Max path sum from any Node
int findMaxSumUtil(Node* root, int& max_sum) {
    
    if(!root)
        return 0;
    if(!root->left && !root->right) {
        if(root->data > max_sum) {
            max_sum = root->data;
        }
        return root->data;
    }
       
    int lsum = findMaxSumUtil(root->left, max_sum);
    int rsum = findMaxSumUtil(root->right, max_sum);
    
    int mx_calc = root->data;
    mx_calc += lsum>0?lsum:0;
    mx_calc += rsum>0?rsum:0;
    
    max_sum = max(max_sum, mx_calc);
    
    return max(root->data, root->data+max(lsum, rsum));
}

// Maximum difference between Node and its ancestor 
int maxDiffUtil(Node* root, int& max_dif) {
    
    if(!root)
        return INT_MAX;
    if(!root->left && !root->right) {
        return root->data;
    }
       
    int lmin = maxDiffUtil(root->left, max_dif);
    int rmin = maxDiffUtil(root->right, max_dif);
    
    max_dif = max(max_dif, (root->data - min(lmin, rmin)));
    
    return min( root->data, min(lmin, rmin)); 
}


// Using level order traversal, you can print left/right/bottom/top view of tree.
vector<int> levelOrder(Node* node)
{
  queue<Node*> q{};
  vector<int> vec{};
  if(!node)
    return vec;
  q.push(node);
  while(!q.empty()) {
      int sz = q.size();
      while(sz--) {
          Node *front = q.front();
          q.pop();
          vec.push_back(front->data);
          if(front->left)
            q.push(front->left);
          if(front->right)
            q.push(front->right);
          
      }
  }
}

// Bottom View of Binary Tree
vector <int> bottomView(Node *node)
{
  // Node:horizontal length
  queue<pair<Node*, int>> q{};
  vector<int> vec{};
  // index:data
  map<int, int> mp{};
  if(!node)
    return vec;
  q.push(make_pair(node, 0));
  mp[0] = node->data;
  while(!q.empty()) {
      int sz = q.size();
      while(sz--) {
          pair<Node*, int> pr = q.front();
          Node *front = pr.first;
          int hd = pr.second;
          mp[hd] = front->data;
          q.pop();
          if(front->left) 
              q.push(make_pair(front->left, hd-1));
          if(front->right)
            q.push(make_pair(front->right, hd+1));
          
      }
  }
  for(auto a: mp) {
      vec.push_back(a.second);
  }
  return vec;
}

void verticalTraversalUtil(Node *node, vector<int>& vec)
{
  // Node:horizontal length
  queue< pair<Node*, int> > q{};
  // index:data
  map<int, vector<int>> mp{};
  if(!node)
    return;
  q.push(make_pair(node, 0));
  while(!q.empty()) {
      int sz = q.size();
      while(sz--) {
          pair<Node*, int> pr = q.front();
          Node *front = pr.first;
          int hd = pr.second;
          mp[hd].push_back(front->data);
          q.pop();
          if(front->left) 
              q.push(make_pair(front->left, hd-1));
          if(front->right)
             q.push(make_pair(front->right, hd+1));
          
      }
  }
  for(auto a: mp)
      for(auto b: a.second)
            vec.push_back(b);
  
  return;
}

Node *getNextRight(Node *p)
{
	Node *temp = p->nextRight;

	/* Traverse Nodes at p's level
	and find and return the first
	Node's first child */
	while (temp != NULL) {
		if (temp->left != NULL)
			return temp->left;
		if (temp->right != NULL)
			return temp->right;
		temp = temp->nextRight;
	}

	// If all the Nodes at p's level
	// are leaf Nodes then return NULL
	return NULL;
}

/* Sets nextRight of all Nodes
of a tree with root as p */
void connectRecur(Node* p) {
	Node *temp;

	if (!p)
        return;

	// Set nextRight for root
	p->nextRight = NULL;

	// set nextRight of all levels one by one
	while (p != NULL) {
		Node *q = p;

		/* Connect all childrem Nodes of p and
		children Nodes of all other Nodes at
		same level as p */
		while (q != NULL) {
			// Set the nextRight pointer
			// for p's left child
			if (q->left) {
				// If q has right child, then
				// right child is nextRight of
				// p and we also need to set
				// nextRight of right child
				if (q->right)
					q->left->nextRight = q->right;
				else
					q->left->nextRight = getNextRight(q);
			}

			if (q->right)
				q->right->nextRight = getNextRight(q);

			// Set nextRight for other
			// Nodes in pre order fashion
			q = q->nextRight;
		}

		// start from the first
		// Node of next level
		if (p->left)
			p = p->left;
		else if (p->right)
			p = p->right;
		else
			p = getNextRight(p);
	}
}

// C++ implementation to print the sequence
// of burning of nodes of a binary tree
#include <bits/stdc++.h>
using namespace std;

// A Tree node
struct Node {
	int key;
	struct Node *left, *right;
};

// Utility function to create a new node
Node* newNode(int key)
{
	Node* temp = new Node;
	temp->key = key;
	temp->left = temp->right = NULL;
	return (temp);
}

// Utility function to print the sequence of burning nodes
int burnTreeUtil(Node* root, int target, queue<Node*>& q)
{
	// Base condition
	if (root == NULL) {
		return 0;
	}

	// Condition to check whether target
	// node is found or not in a tree
	if (root->key == target) {
		cout << root->key << endl;
		if (root->left != NULL) {
			q.push(root->left);
		}
		if (root->right != NULL) {

			q.push(root->right);
		}

		// Return statements to prevent
		// further function calls
		return 1;
	}

	int a = burnTreeUtil(root->left, target, q);

	if (a == 1) {
		int qsize = q.size();

		// Run while loop until size of queue
		// becomes zero
		while (qsize--) {
			Node* temp = q.front();

			// Printing of burning nodes
			cout << temp->key << " , ";
			q.pop();

			// Check if condition for left subtree
			if (temp->left != NULL)
				q.push(temp->left);

			// Check if condition for right subtree
			if (temp->right != NULL)
				q.push(temp->right);
		}

		if (root->right != NULL)
			q.push(root->right);

		cout << root->key << endl;

		// Return statement it prevents further
		// further function call
		return 1;
	}

	int b = burnTreeUtil(root->right, target, q);

	if (b == 1) {
		int qsize = q.size();
		// Run while loop until size of queue
		// becomes zero

		while (qsize--) {
			Node* temp = q.front();

			// Printing of burning nodes
			cout << temp->key << " , ";
			q.pop();

			// Check if condition for left subtree
			if (temp->left != NULL)
				q.push(temp->left);

			// Check if condition for left subtree
			if (temp->right != NULL)
				q.push(temp->right);
		}

		if (root->left != NULL)
			q.push(root->left);

		cout << root->key << endl;

		// Return statement it prevents further
		// further function call
		return 1;
	}
}

// construct-ancestor-matrix-from-a-given-binary-tree
// 'n' is tree size
void ancestorMatrix(Node *root, int n)
{
	if (!root) return;
    
    ancestorMatrix(root->left, n); //Process left child
    ancestorMatrix(root->right, n);//Process right child
    
    /* mat[root->data] row will be ancestor of all of its childs.
       mat[root->left->child] will store nodes for which root->left is ancestor
       so, copy mat[root->left->child] row of matrix to mat[root->data] row
    */
    if (root->left) {
        for (int i = 0; i < n; i++)
            mat[root->data][i] =  mat[root->left->data][i];
        mat[root->data][root->left->data] = 1; // root is ancestor of its child
    }
    if (root->right) {
        for (int i = 0; i < n; i++)
            mat[root->data][i] |=  mat[root->right->data][i];
        mat[root->data][root->right->data] = 1;        
    }
}

// Given a binary tree and target node. By giving the fire to the target node and fire 
// starts to spread in a complete tree. The task is to print the sequence of the burning
// nodes of a binary tree.
void burnTree(Node* root, int target) {
	queue<Node*> q;

	// Function call
	burnTreeUtil(root, target, q);

	// While loop runs unless queue becomes empty
	while (!q.empty()) {
		int qSize = q.size();
		while (qSize > 0) {
			Node* temp = q.front();

			// Printing of burning nodes
			cout << temp->key;
			// Insert left child in a queue, if exist
			if (temp->left != NULL) {
				q.push(temp->left);
			}
			// Insert right child in a queue, if exist
			if (temp->right != NULL) {
				q.push(temp->right);
			}

			if (q.size() != 1)
				cout << " , ";

			q.pop();
			qSize--;
		}
		cout << endl;
	}
}

struct Node* treeCreator()
{
	//create the main pointer
	struct Node *mainPtr;
	mainPtr = new Node(10);
	mainPtr->left = new Node(6);
	mainPtr->right = new Node(12);

	mainPtr->left->left = new Node(2);
	mainPtr->left->right = new Node(9);

	mainPtr->right->left = new Node(11);
	mainPtr->right->right = new Node(18);

	mainPtr->right->right->left = new Node(16);
	mainPtr->right->right->right = new Node(20);

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


int main() {
    Node *root = treeCreator();
    Node *res = lca(root , 11, 0);
    if(res)
        cout<<"Result is: "<<res->data<<endl;
    else
        cout<<"No match found"<<endl;
    return 0;
}


// Notes:
// To burn the binary tree starting from the target node
/// take 2 variables a and b, a being returned by left subtree and b being returned by right subtree.
/// We will use queue for this and for down side we will use normal level -order